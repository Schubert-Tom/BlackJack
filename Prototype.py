from __future__ import annotations
from functools import lru_cache
from typing import List
from random import shuffle
from timeit import default_timer
from collections import Counter
from fractions import Fraction


# Rules
HIT_ON_SOFT_17 = False
DOUBLE_AFTER_SPLIT = False
SPLIT_AFTER_SPLIT = False
BJ_PROOF_IMMEDIATELY = True
HIT_ON_SPLIT_ACES = False
DECKS = 1

# Funktion zur Bestimmung der Laufzeit


def timer(func):
    def time(*args, **kwargs):
        start = default_timer()
        returnVal = func(*args, **kwargs)
        end = default_timer()
        print(f"Function {func.__name__} needs {end-start} s to run")
        return returnVal
    return time

# Calculates the EW for a given Card Distribution and given Dealer anf Player HandValues


class Distribution():
    """
    Class represents the card distribution
    """
    prob_distribution: tuple
    card_distribution: list
    amount_of_Cards: int

    def __init__(self, card_distribution: tuple, amount_of_Cards: int):
        self.amount_of_Cards = amount_of_Cards
        self.card_distribution = card_distribution
        self.prob_distribution = tuple(
            [Fraction(count, self.amount_of_Cards) for count in card_distribution])

    @classmethod
    def fromList(cls, liste: List[int]) -> Distribution:
        amount_of_Cards = len(liste)
        count = Counter(liste)
        cards = [0] * 10
        for x in count.keys():
            cards[x-2] += count[x]
        return cls(card_distribution=cards, amount_of_Cards=amount_of_Cards)

    def removeCard(self, card) -> Distribution:
        """
        Create a Distribution with one Card less
        """
        new_dist = list(self.card_distribution)
        new_dist[card-2] -= 1
        return Distribution(card_distribution=tuple(new_dist), amount_of_Cards=self.amount_of_Cards-1)

    def __repr__(self):
        return f"{self.prob_distribution} : {self.amount_of_Cards}"

    def __hash__(self):
        return hash(self.prob_distribution)

    def __eq__(self, other):
        return isinstance(other, Distribution) and self.card_distribution == other.card_distribution


class Hand():
    """
    class represents a Player or Dealers HandCards
    """
    handCards: tuple
    value: int
    afterSplit: bool
    soft: bool

    def __init__(self, cards, afterSplit=False):
        self.handCards = tuple(sorted(cards))
        self.value, self.soft = self._calcHandValue()
        self.afterSplit = afterSplit

    def __repr__(self):
        return f"{self.handCards} : {self.value}, {self.soft}"

    def __hash__(self):
        return hash(self.handCards+tuple([self.afterSplit]))

    def __eq__(self, other):
        return isinstance(other, Hand) and self.value == other.value and self.afterSplit == other.afterSplit

    def _calcHandValue(self):
        softhand = 11 in self.handCards
        val = sum(self.handCards)
        if val > 21 and 11 in self.handCards:
            for card in self.handCards:
                if val > 21 and card == 11:
                    val -= 10
                    softhand = False
                elif card == 11:
                    softhand = True
        return val, softhand

    def isBlackJack(self) -> bool:
        if len(self.handCards) == 2 and set(self.handCards) == set([11, 10]) and self.afterSplit == False:
            return True
        return False


def eval(playerHand: Hand, dealerHand: Hand) -> List[Fraction]:
    """
    Function returns winning factor

    1. Check for BlackJAck draw
    2. Check for Dealer Black JAck
    3. Check for Player Black JAck
    4. Check for Player Bust
    5. Check for Dealer Bust
    6. Check which cards higher
    7. Draw
    """

    if dealerHand.isBlackJack() and playerHand.isBlackJack():
        return [Fraction(0), Fraction(1, 1), Fraction(0)]
    elif dealerHand.isBlackJack():
        return [Fraction(1), Fraction(0), Fraction(0)]
    if playerHand.isBlackJack():
        return [Fraction(0), Fraction(0), Fraction(3, 2)]
    if playerHand.value > 21:
        return [Fraction(1), Fraction(0), Fraction(0)]
    if dealerHand.value > 21:
        return [Fraction(0), Fraction(0), Fraction(1)]
    if dealerHand.value > playerHand.value:
        return [Fraction(1), Fraction(0), Fraction(0)]
    if dealerHand.value < playerHand.value:
        return [Fraction(0), Fraction(0), Fraction(1)]
    return [Fraction(0), Fraction(1), Fraction(0)]


@lru_cache(maxsize=1024)
def EW(PlayerVal: Hand, DealerVal: Hand, distribution: Distribution, dealernextCardnoTen=False):
    EW_Hit = EW_Stand = EW_Double = EW_Split = -100
    # IF Dealer shows 11 --> BJ check already done --> No ten
    if BJ_PROOF_IMMEDIATELY == True and DealerVal.handCards == tuple([11]):
        dealernextCardnoTen = True
    else:
        dealernextCardnoTen = False
    # Stand
    stand = DealerWinProb(PlayerVal=PlayerVal, DealerVal=DealerVal,
                          distribution=distribution, dealernextCardnoTen=dealernextCardnoTen)
    EW_Stand = stand[0]*-1+stand[1]*0+stand[2]*1

    # Calculate all Hit-Options with perfect gameplay
    if PlayerVal.value < 21:
        EW_Hit = 0
        for card_val, card_count in enumerate(distribution.card_distribution):
            # Index to Value 0:2,..,9:A
            card_val += 2
            # Skip not available Cards
            if card_count == 0:
                continue
            newPlayerVal = Hand(PlayerVal.handCards + tuple([card_val]))
            new_distribution = distribution.removeCard(card_val)
            EW_Hit += distribution.prob_distribution[card_val-2] * EW(
                PlayerVal=newPlayerVal, DealerVal=DealerVal, distribution=new_distribution, dealernextCardnoTen=dealernextCardnoTen)[0]

    # Double Down
    # Check if double allowed
    if PlayerVal.afterSplit == False or (PlayerVal.afterSplit == True and DOUBLE_AFTER_SPLIT == True):
        if len(PlayerVal.handCards) == 2:
            if PlayerVal.value < 21:
                double = [Fraction(0), Fraction(0), Fraction(0)]
                for card_val, card_count in enumerate(distribution.card_distribution):
                    # Index to Value 0:2,..,9:A
                    card_val += 2
                    if card_count == 0:
                        continue
                    newPlayerVal = Hand(
                        PlayerVal.handCards + tuple([card_val]), afterSplit=True)
                    new_distribution = distribution.removeCard(card_val)

                    dealerwinprob = DealerWinProb(
                        newPlayerVal, DealerVal, new_distribution, dealernextCardnoTen=dealernextCardnoTen)
                    double[0] += distribution.prob_distribution[card_val -
                                                                2] * dealerwinprob[0]
                    double[1] += distribution.prob_distribution[card_val -
                                                                2] * dealerwinprob[1]
                    double[2] += distribution.prob_distribution[card_val -
                                                                2] * dealerwinprob[2]

                EW_Double = double[0]*-2 + double[1]*0 + double[2]*2

    # Split
    # Split Combination Calculation between Split Pairs is not included. Simply Doubling the expected Value seems to be enough
    # Check if Split is allowed
    if len(PlayerVal.handCards) == 2 and PlayerVal.handCards[0] == PlayerVal.handCards[1]:
        if PlayerVal.afterSplit == False or (PlayerVal.afterSplit == True and SPLIT_AFTER_SPLIT == True):
            if PlayerVal.value < 21:
                EW_Split = 0.0
                # All cards but aces --> special case see Rules
                if PlayerVal.handCards[0] != 11 or HIT_ON_SPLIT_ACES == True:
                    for card_val, card_count in enumerate(distribution.card_distribution):
                        # Index to Value 0:2,..,9:A
                        card_val += 2
                        if card_count == 0:
                            continue
                        newPlayerVal = Hand(
                            tuple(PlayerVal.handCards[:1]) + tuple([card_val]), afterSplit=True)
                        new_distribution = distribution.removeCard(card_val)
                        EW_Split += 2*distribution.prob_distribution[card_val-2] * EW(
                            PlayerVal=newPlayerVal, DealerVal=DealerVal, distribution=new_distribution, dealernextCardnoTen=dealernextCardnoTen)[0]
                else:
                    # Aces
                    split = [Fraction(0), Fraction(0), Fraction(0)]
                    for card_val, card_count in enumerate(distribution.card_distribution):
                        # Index to Value 0:2,..,9:A
                        card_val += 2
                        if card_count == 0:
                            continue
                        newPlayerVal = Hand(
                            tuple(PlayerVal.handCards[:1]) + tuple([card_val]), afterSplit=True)
                        new_distribution = distribution.removeCard(card_val)
                        dealerwinprob = DealerWinProb(
                            newPlayerVal, DealerVal, new_distribution, dealernextCardnoTen=dealernextCardnoTen)
                        split[0] += distribution.prob_distribution[card_val -
                                                                   2] * dealerwinprob[0]
                        split[1] += distribution.prob_distribution[card_val -
                                                                   2] * dealerwinprob[1]
                        split[2] += distribution.prob_distribution[card_val -
                                                                   2] * dealerwinprob[2]

                    EW_Split = split[0]*-2 + split[1]*0 + split[2]*2

    # Evaluation
    decision = {
        0: "STAND",
        1: "HIT",
        2: "DOUBLE",
        3: "SPLIT"
    }
    options = [EW_Stand, EW_Hit, EW_Double, EW_Split]
    def f(i): return options[i]
    # returns the expected Value and the decision made example: -0.42323, "STAND"
    return max(options), decision[max(range(len(options)), key=f)]


@lru_cache(maxsize=None)
def DealerWinProb(PlayerVal, DealerVal: Hand, distribution: Distribution, dealernextCardnoTen=False):
    """
    -> returns [Fraction, Fraction, Fraction]
    0.: Dealer Win Propability
    1.: Draw Probability
    2.: Player Win Probability               
    """
    # Dealer ppeks for BJ (US-Version) --> cancel probability for value 10 on next move
    if dealernextCardnoTen == True:
        cards = list(distribution.card_distribution)
        cards[8] = 0
        modified_distribution = Distribution(
            card_distribution=cards, amount_of_Cards=sum(cards))
    # Dealer Stands by rule
    if DealerVal.value > 17:
        return eval(PlayerVal, DealerVal)
    elif DealerVal.value == 17 and DealerVal.soft == False:
        return eval(PlayerVal, DealerVal)
    elif DealerVal.value == 17 and DealerVal.soft == True and HIT_ON_SOFT_17 == False:
        return eval(PlayerVal, DealerVal)

    # Dealer hits by rule
    probability = [Fraction(0), Fraction(0), Fraction(0)]
    for card_val, card_count in enumerate(distribution.card_distribution):
        # Index to Value 0:2,..,9:A
        card_val += 2
        if card_count == 0:
            continue
        if dealernextCardnoTen == True and card_val == 10:
            continue

        newDealerVal = Hand(DealerVal.handCards + tuple([card_val]))
        new_distribution = distribution.removeCard(card_val)
        dealerwinprob = DealerWinProb(
            PlayerVal, newDealerVal, new_distribution, dealernextCardnoTen=False)
        if dealernextCardnoTen == True:
            probability[0] += modified_distribution.prob_distribution[card_val -
                                                                      2] * dealerwinprob[0]
            probability[1] += modified_distribution.prob_distribution[card_val -
                                                                      2] * dealerwinprob[1]
            probability[2] += modified_distribution.prob_distribution[card_val -
                                                                      2] * dealerwinprob[2]
        else:
            probability[0] += distribution.prob_distribution[card_val -
                                                             2] * dealerwinprob[0]
            probability[1] += distribution.prob_distribution[card_val -
                                                             2] * dealerwinprob[1]
            probability[2] += distribution.prob_distribution[card_val -
                                                             2] * dealerwinprob[2]
    return probability


@timer
def test():
    basicdistr = [2]*4*DECKS + [3]*4*DECKS + [4]*4*DECKS + [5]*4*DECKS + [6]*4 * \
        DECKS + [7]*4*DECKS + [8]*4*DECKS + [9] * \
        4*DECKS + [10]*16*DECKS + [11]*4*DECKS
    shuffle(basicdistr)
    basicdistr = Distribution.fromList(basicdistr)

    f = EW(Hand([10, 8]), Hand([6]), basicdistr)
    print(float(f[0]), f[1])
    print(DealerWinProb.cache_info())
    print(EW.cache_info())


if __name__ == "__main__":
    test()
