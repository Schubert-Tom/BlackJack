# BlackJack Card Counting

This repository does show an implementation for calculating the expected value in a game of black jack based on perfect play.
Therefore the card distribution, handcards of the player and the dealer's face card are taken into account for accurate calculation
of the expected value on each game decision. A min-max Strategy is used to traverse a probabilty tree and find optimal decisions based on the given
parameters.

Through counting the dealt cards and observing the distribution of still available cards a player can minimize the dealers advantage and maybe even 
tilt it in his favor.

## Prototype 

The prototype of the calculation is written in python and does implement a recursive depth first search probability tree traversal.
Hands and available card distributions are represented by a class.
To avoid double calculation of different shoe-hand combinations a lru-cache system is implemented for the functions.

## C++ Implementation

The fast c++ implementation does follow the same recursive calculation scheme like the protottype.
For faster calculation better memory handling and more efficient data structures are used.
The calculation does still traverse the whole propability tree by a recursive scheme and no simplifications are made in the card distribution.
This does allow more exact decision making especially on small shoes and less available cards
The faster c++ Implementation does not support caching yet.
Additionally a thread based load balancer system could make the calculation much faster but is also still not implemented.

## Game-Settings

Setting  | Default | Meaning
:-------------: | :-------------: | :-------------:
HIT_ON_SOFT_17  | false | if true dealer will hit on soft 17 hands.
DOUBLE_AFTER_SPLIT  | false | if true layer is allowed to play double down on splitted cards
SPLIT_AFTER_SPLIT  | false | if true multiple splits are allowed.
BJ_PROOF_IMMEDIATELY_ON_ACE  | true | if true the dealer checks for a black jack when an ace is shown
HIT_ON_SPLIT_ACES | false | if true the player is allowed to hit on splitted aces
DECKS  | 1 | How many cards are inside the shoe
BlackJackPayout  | 1.5  | The multiple for the payout if the player gets a blackjack
CALCSPLIT | true  | If the option of splits should be calculated in general.

## Timing

The prototype and implementation do both support time measurement for evaluating the calculation.
Although the implementation has no caching, it is still up to 15 times faster than the computation of the prototype.
Complex computations like dealer shows 2 and Player shows 2,2 do need quite amount of time on large decks because of the tree complexity.


The software should only demonstrate on how to perform probabilty calculation in the game of blackjack correctly without simplifications.
More efficient computations do apply simplifications like a constant distribution while calculating card combination probabilities.

## Disclaimer

This software should bot be used to play against casinos for two reasons.

1. The calculation does need too much time for proper a proper game integration
2. No Warranty for the correctness of the calculation and according recommandation
3. The casion always wins
