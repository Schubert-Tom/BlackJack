/**
Calculate the BJ expected Value based on the Players Hand
TODO:   1. Check for possible failure in calucaltions (difference between python and c++)
        2. Introduce caching
        3. Make multiple threads for calculation
*/
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <chrono>
#include "header/Hand.hpp"
#include "header/Distribution.hpp"

/*
Game Constants
*/

#define HIT_ON_SOFT_17 false
#define DOUBLE_AFTER_SPLIT false
#define SPLIT_AFTER_SPLIT false
#define BJ_PROOF_IMMEDIATELY_ON_ACE true
#define HIT_ON_SPLIT_ACES false
#define DECKS 1
#define BlackJackPayout 1.5
#define CALCSPLIT true
#define FULLDECK DECKS * 52

#define TYPE double

uint nodes = 0;
uint cummunodes = 0;

/**
 * \brief Evaluate the outcome of a game and return the money multiple
 * \param[in] player Players Hand
 * \param[in] dealer Dealers Hand
 * \return The Multiple of the game outcome. If the dealer
 * wins the Player loses all of his money --> -1.
 * If the Player wins he will double his money --> 1
 * expected value of 0 --> Both Players have the same chance for winning
 */

struct PlayRecommendation
{
    std::string recommendation;
    TYPE ev;
};

enum Options
{
    stand = 0,
    hit,
    doubledown,
    split
};

/**
 * \brief Evaluate the outcome of a play and return the expected Value
 */

PlayRecommendation eval(const Hand &player, const Hand &dealer)
{
    if (player.blackJack == true && dealer.blackJack == true)
        return {"", 0.0};
    else if (dealer.blackJack)
        return {"", -1.0};
    else if (player.blackJack)
        return {"", BlackJackPayout};
    else if (player.value > 21)
        return {"", -1.0};
    else if (dealer.value > 21)
        return {"", 1.0};
    else if (dealer.value > player.value)
        return {"", -1.0};
    else if (dealer.value < player.value)
        return {"", 1.0};
    else
        return {"", 0.0};
};

/**
 * \brief Traverse a probability tree with depth first search and
 * optimal play on each level.
 */
TYPE treeTraversal(Hand &changedhand, Hand &samehand, Distribution &dist, PlayRecommendation (*func)(Hand &, Hand &, Distribution &))
{
    TYPE EW = 0.0;
    int possiblyAvailableCardsInShoe = 0;
    for (auto idx = 0; idx < dist.dist.size(); idx++)
    {
        std::chrono::_V2::system_clock::time_point t1, t2;
        if (dist.amountOfCards == FULLDECK)
            t1 = std::chrono::high_resolution_clock::now();
        if (dist.dist[idx] == 0)
            continue;
        int cardVal = idx + 2;
        changedhand.appendCard(cardVal);
        possiblyAvailableCardsInShoe += dist.dist[idx];
        dist.removeCard(cardVal);
        EW += (dist.dist[idx] + 1) * func(changedhand, samehand, dist).ev;
        // Restore original Distribution and Hand Object without changes
        changedhand.removeCard(cardVal);
        dist.appendCard(cardVal);
        if (dist.amountOfCards == FULLDECK)
        {
            t2 = std::chrono::high_resolution_clock::now();
            // Getting number of milliseconds as a double.
            std::chrono::duration<double, std::milli> ms_double = t2 - t1;
            std::cout << (ms_double.count() / 1000) << " seconds for " << idx + 2 << "\n";
            std::cout << nodes << " Nodes parsed in probability tree\n";
            nodes = 0;
        }
    }
    return EW /= possiblyAvailableCardsInShoe;
};

/**
 * \brief Calculate the expected Value when Player makes no more moves
 */
PlayRecommendation dealerWinEP(Hand &playerVal, Hand &dealerVal, Distribution &dist)
{
    // Dealer Stands
    cummunodes++;
    nodes++;
    if (dealerVal.value > 17)
    {
        return eval(playerVal, dealerVal);
    }
    else if (dealerVal.value == 17 && dealerVal.soft == false)
    {
        return eval(playerVal, dealerVal);
    }
    else if (!HIT_ON_SOFT_17 && dealerVal.value == 17 && dealerVal.soft == true)
    {
        return eval(playerVal, dealerVal);
    }

    TYPE expectedValue = 0.0;
    int possiblyAvailableCardsInShoe = 0;
    for (auto idx = 0; idx < dist.dist.size(); idx++)
    {
        if (dist.dist[idx] == 0)
            continue;
        int cardVal = idx + 2;
        // Skip ten when dealer peeks for bj (US)
        if (BJ_PROOF_IMMEDIATELY_ON_ACE && dealerVal.value == 11 && dealerVal.amountofCards == 1 && cardVal == 10)
            continue;
        // Manipulate Distribution and Hand Object for further calculation
        dealerVal.appendCard(cardVal);
        possiblyAvailableCardsInShoe += dist.dist[idx];
        dist.removeCard(cardVal);
        // ep = CardProb * ep(,playerVal, DealerVal modifiedDist)
        expectedValue += (dist.dist[idx] + 1) * dealerWinEP(playerVal, dealerVal, dist).ev;
        // Restore original Distribution and Hand Object without changes
        dealerVal.removeCard(cardVal);
        dist.appendCard(cardVal);
    }
    return {"", expectedValue / possiblyAvailableCardsInShoe};
};

/**
 * \brief Calculate the expected Value on all different Hand possibilities
 */
PlayRecommendation ev(Hand &playerVal, Hand &dealerVal, Distribution &dist)
{
    nodes++;
    cummunodes++;
    TYPE EWHit = -100.0, EWStand = -100.0, EWDouble = -100.0, EWSplit = -100.0;
    // Stand Calculation
    std::chrono::_V2::system_clock::time_point t1, t2;
    if (dist.amountOfCards == FULLDECK)
        t1 = std::chrono::high_resolution_clock::now();

    EWStand = dealerWinEP(playerVal, dealerVal, dist).ev;

    if (dist.amountOfCards == FULLDECK)
    {
        t2 = std::chrono::high_resolution_clock::now();
        // Getting number of milliseconds as a double.
        std::chrono::duration<double, std::milli> ms_double = t2 - t1;
        std::cout << (ms_double.count() / 1000) << " seconds for Stand-Calculation \n\n\n";
        nodes = 0;
    }
    // Hit Calculation
    if (playerVal.value < 21)
    {
        if (dist.amountOfCards == FULLDECK)
            t1 = std::chrono::high_resolution_clock::now();

        EWHit = treeTraversal(playerVal, dealerVal, dist, ev);

        if (dist.amountOfCards == FULLDECK)
        {
            t2 = std::chrono::high_resolution_clock::now();
            // Getting number of milliseconds as a double.
            std::chrono::duration<double, std::milli> ms_double = t2 - t1;
            std::cout << (ms_double.count() / 1000) << " seconds for HIT-Calculation \n\n\n";
        }
        // Double Calculation
        if ((DOUBLE_AFTER_SPLIT && playerVal.afterSplit) || !playerVal.afterSplit)
        {
            if (playerVal.amountofCards == 2)
            {
                if (dist.amountOfCards == FULLDECK)
                    t1 = std::chrono::high_resolution_clock::now();

                EWDouble = 2 * treeTraversal(playerVal, dealerVal, dist, dealerWinEP);

                if (dist.amountOfCards == FULLDECK)
                {
                    t2 = std::chrono::high_resolution_clock::now();
                    // Getting number of milliseconds as a double.
                    std::chrono::duration<double, std::milli> ms_double = t2 - t1;
                    std::cout << (ms_double.count() / 1000) << " seconds for Double-Calculation \n\n\n";
                }
            }
        }
        // Split Calculation
        if (CALCSPLIT && playerVal.splitable != 0)
        {
            if ((SPLIT_AFTER_SPLIT && playerVal.afterSplit) || playerVal.afterSplit == false)
            {
                if (dist.amountOfCards == FULLDECK)
                    t1 = std::chrono::high_resolution_clock::now();

                playerVal.removeCard(playerVal.splitable);
                playerVal.afterSplit = true;
                // Treat ACE-Split differently
                if (!HIT_ON_SPLIT_ACES && playerVal.value == 11 && playerVal.amountofCards == 1)
                    EWSplit = 2 * treeTraversal(playerVal, dealerVal, dist, dealerWinEP);
                else
                    EWSplit = 2 * treeTraversal(playerVal, dealerVal, dist, ev);

                playerVal.appendCard(playerVal.splitable);
                playerVal.afterSplit = false;

                if (dist.amountOfCards == FULLDECK)
                {
                    t2 = std::chrono::high_resolution_clock::now();
                    // Getting number of milliseconds as a double.
                    std::chrono::duration<double, std::milli> ms_double = t2 - t1;
                    std::cout << (ms_double.count() / 1000) << " seconds for Split-Calculation \n\n\n";
                }
            }
        }
    }
    // Available Options
    std::array<TYPE, 4> options = std::array{EWStand, EWHit, EWDouble, EWSplit};
    TYPE *ev = std::max_element(options.begin(), options.end());
    Options selectedOption = (Options)std::distance(options.begin(), ev);

    // Choose best Option
    switch (selectedOption)
    {
    case hit:
        return {"Hit", *ev};
    case stand:
        return {"Stand", *ev};
    case doubledown:
        return {"Double", *ev};
    case split:
        return {"Split", *ev};
    }
};

/**
 * \brief Function shows usage of ev-function
 */
void test1()
{
    Distribution dist = Distribution(std::array{4 * DECKS, 4 * DECKS, 4 * DECKS, 4 * DECKS, 4 * DECKS, 4 * DECKS, 4 * DECKS, 4 * DECKS, 16 * DECKS, 4 * DECKS});
    Hand player = Hand({8, 10});
    std::cout << player << std::endl;
    Hand dealer = Hand({6});
    PlayRecommendation evalue = ev(player, dealer, dist);
    std::cout << std::setprecision(std::numeric_limits<double>::max_digits10) << evalue.ev << " expected value when " << evalue.recommendation << std::endl;
}

int main()
{
    auto t1 = std::chrono::high_resolution_clock::now();
    test1();
    auto t2 = std::chrono::high_resolution_clock::now();
    // Getting number of milliseconds as a double.
    std::chrono::duration<double, std::milli> ms_double = t2 - t1;
    std::cout << (ms_double.count() / 1000) << " seconds\n";
    std::cout << cummunodes << " Nodes parsed in general probability tree\n";
    return 0;
}