#pragma once
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <array>
#include <numeric>

class Distribution{
private:

    std::array<int,10> dist_ = {0,0,0,0,0,0,0,0,0,0};
    double amountOfCards_; 

public:
    const decltype(dist_)& dist = dist_;
    const decltype(amountOfCards_)& amountOfCards = amountOfCards_;

    /**
     * \brief Constructor for creating a distribution based of an Card Vector
     * \param[in] cardList Available Cards in the shoe
    */
    Distribution(const std::vector<int>& cards)
    {
        for(const int cardVal: cards) dist_[cardVal-2] +=1;
        amountOfCards_ = cards.size();
    }
    /**
     * \brief Constructor for creating a distribution-Object based of a given Distribution
     * Using Member Initalizer Lists to avoid array construction made twice.
     * \param[in] distribution Card distribution of the shoe 
     * 
    */
    Distribution(const std::array<int, 10>& cardDist)
        : dist_(cardDist), amountOfCards_(std::accumulate(begin(cardDist), end(cardDist), 0))
    {}

    void removeCard(int card){
        dist_[card-2] --;
        amountOfCards_--;
    }

    void appendCard(int card){
        dist_[card-2] ++;
        amountOfCards_++;
    }

    friend std::ostream& operator<<(std::ostream& os, const Distribution& dist);
};

std::ostream& operator<<(std::ostream& os, const Distribution& dist){
    for (auto val = 0; val < dist.dist.size(); val++)
        os << val << ":" << dist.dist[val] << "; "; 
    return os;
}