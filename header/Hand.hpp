#pragma once
#include <vector>
#include <iostream>
#include <array>


class Hand
{
private:
    std::array<int,10> cards_ = {0,0,0,0,0,0,0,0,0,0};
    int value_ = 0;
    bool soft_ = false;
    int amountofCards_;
    bool blackJack_ = false;
    int splitable_ = 0;

    void calcValue(){
        value_=0;
        soft_ = false;
        splitable_ = 0;
        for (int card = 0; card < 10;card++){
            value_ += cards_[card]*(card+2);
            if(amountofCards_ == 2 && cards_[card] == 2) splitable_ = card+2;  
        }
        for (int ace_val = 0; ace_val < this->cards_[9]; ace_val++){
            soft_ = true;
            if(value_ > 21){
                value_ -= 10;
                soft_ = false;
            }
        }
        if (value_ == 21 && amountofCards_ == 2 && afterSplit == false){
            blackJack_ = true;
        }
        else{
            blackJack_ = false;
        }
    }

public:
    bool afterSplit = false;
    
    Hand(const std::vector<int>& handCards){
        for (int card: handCards){
            if(card < 2 || card > 11) throw std::invalid_argument("A card with the value does not exist");
            cards_[card-2]++;
        }
        amountofCards_ = handCards.size();
        calcValue();
    }
    Hand()
        :value_(0), soft_(false), amountofCards_(0), blackJack_(false), splitable_(0)
    {}

    // Declare Read Only attributes as const reference to org value --> not modifiable private members
    // public-scope: read only; private-scope: read&write 
    const decltype(cards_)& cards = cards_;
    const decltype(value_)& value = value_;
    const decltype(soft_)& soft  = soft_;
    const decltype(amountofCards_)& amountofCards = amountofCards_;
    const decltype(blackJack_)& blackJack = blackJack_;
    const decltype(splitable_)& splitable = splitable_;
    
    void appendCard(int cardval){
        cards_[cardval-2] ++;
        amountofCards_ ++;
        calcValue();
    }

    void removeCard(int cardval){
        /*
        Removing a card can not lead to a black jack
        */
        cards_[cardval-2] --;
        amountofCards_ --;
        calcValue();
    }

    friend std::ostream& operator<<(std::ostream& os, const Hand& hand);
};

 std::ostream& operator<<(std::ostream& os, const Hand& hand){
        os << std::boolalpha;   
        os << "Hand with Value " << hand.value;
        os <<", soft=" << hand.soft;
        os << " and blackJack=" << hand.blackJack ;
        os << " and " << hand.amountofCards << " Cards";
        os  << std::endl;
        return os;
    }