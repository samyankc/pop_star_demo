#include <array>
#include <iostream>

using std::cout, std::cin;

template <typename BaseRange>
struct Reverse
{
    const BaseRange& Base;
    auto begin() const { return std::rbegin( Base ); }
    auto end() const { return std::rend( Base ); }
};

enum Colour { EMPTY, R, G, B, P, Y };

constexpr auto RowSize   = 10;
constexpr auto BoardSize = 10;

using Row = std::array<Colour, RowSize>;
struct Board : std::array<Row, BoardSize>
{
    auto RandomizedSetup()
    {
        for( auto& Row : *this )
            for( auto& Cell : Row )
                Cell = ( Colour )( rand() % 5 + 1 );
    }
    
    auto& At(auto r, auto c)
    {
        return ( *this )[ r ][ c ];
    }

    auto LinkedToRight( int r, int c )
    {
        if( c + 1 >= RowSize ) return false;
        return At( r, c ) == At( r, c + 1 );
    }

    auto LinkedToAbove( int r, int c )
    {
        if( r + 1 >= BoardSize ) return false;
        return At( r, c ) == At( r + 1, c );
    }

    auto Display()
    {
        for( auto r = BoardSize - 1; auto& Row : Reverse( *this ) )
        {
            cout << " ";
            for( auto c = 0; auto& Cell : Row )
                cout << ' ' << ( LinkedToAbove( r, c++ ) ? " + " : "   " );
            cout << '\n'
                 << r << "| ";
            for( auto c = 0; auto& Cell : Row )
                cout << Cell << ( LinkedToRight( r, c++ ) ? " + " : "   " );
            cout << '\n';
            
            --r;
        }

        cout << "  ___ ___ ___ ___ ___ ___ ___ ___ ___ ___\n   ";
        for( auto col = 0; col < BoardSize; ++col ) cout << col << "   ";
    }
};

int main()
{
    Board Game;
    Game.RandomizedSetup();
    Game.Display();
}