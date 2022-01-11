#include <array>
#include <iomanip>
#include <iostream>
#include <random>
#include <windows.h>

using std::cout, std::cin;
// clang-format off
enum struct Colour : unsigned int { EMPTY, R, G, B, P, Y };
// clang-format on

constexpr auto ColumnSize = 10u;
constexpr auto RowSize    = 10u;
constexpr auto Max_x      = RowSize;
constexpr auto Max_y      = ColumnSize;

const static auto hWind = GetStdHandle( STD_OUTPUT_HANDLE );
struct ColourBlock
{
    unsigned char RGB : 3;
    friend std::ostream& operator<<( std::ostream& out, ColourBlock Block )
    {
        out << std::flush;
        SetConsoleTextAttribute( hWind, Block.RGB << 4 );
        WriteConsole( hWind, "  ", 2, NULL, NULL );
        SetConsoleTextAttribute( hWind, 0b111 );
        return out;
    }
};

struct Coordinate
{
    unsigned int x, y;
};

auto RandomInput = [ RandomGenerator = std::mt19937{ std::random_device{}() },
                     Distribution    = std::uniform_int_distribution<unsigned int>  //
                     { 0u, std::max( ColumnSize, RowSize ) - 1 } ] mutable          //
{
    return Coordinate{ Distribution( RandomGenerator ), Distribution( RandomGenerator ) };
};

auto UserInput()
{
    cout << "Choose a cell: " << std::flush;
    auto Result = Coordinate{};
    cin >> Result.x >> Result.y;
    return Result;
}

using Column = std::array<Colour, ColumnSize>;
struct Board : std::array<Column, RowSize>
{
    Board()
    {
        auto RandomGenerator = std::mt19937{ std::random_device{}() };
        auto Distribution    = std::uniform_int_distribution<std::underlying_type_t<Colour>>{ std::to_underlying( Colour::R ), std::to_underlying( Colour::Y ) };
        for( auto& Column : *this )
            for( auto& Cell : Column )
                Cell = static_cast<Colour>( Distribution( RandomGenerator ) );
    }

    auto& At( auto x ) { return ( *this )[ x ]; }
    auto& At( auto x, auto y ) { return At( x )[ y ]; }
    const auto& At( auto x ) const { return ( *this )[ x ]; }
    const auto& At( auto x, auto y ) const { return At( x )[ y ]; }

    auto Empty( auto x, auto y ) const { return At( x, y ) == Colour::EMPTY; }


    #define ToRight +1, 0
    #define ToLeft  -1, 0
    #define ToAbove 0, +1
    #define ToBelow 0, -1
    template <auto CHECK_LR, auto CHECK_AB>
    auto SafeBound( auto x, auto y ) const
    {
        if constexpr( CHECK_LR == +1 ) if( x >= Max_x - 1 ) return false;
        if constexpr( CHECK_AB == +1 ) if( y >= Max_y - 1 ) return false;
        if constexpr( CHECK_LR == -1 ) if( x == 0 ) return false;
        if constexpr( CHECK_AB == -1 ) if( y == 0 ) return false;
        return true;
    }

    template <auto CHECK_LR, auto CHECK_AB>
    auto Linked( auto x, auto y ) const
    {
        return ! Empty( x, y ) &&
               SafeBound<CHECK_LR, CHECK_AB>( x, y ) &&
               At( x, y ) == At( x + CHECK_LR, y + CHECK_AB );
    }

    template <auto CHECK_LR, auto CHECK_AB>
    auto Matched( auto x, auto y, auto TargetColour ) const
    {
        return SafeBound<CHECK_LR, CHECK_AB>( x, y ) &&
               TargetColour == At( x + CHECK_LR, y + CHECK_AB );
    }

    auto Exhausted() const
    {
        auto Result = true;
        for( auto x = 0; x < Max_x; ++x )
            for( auto y = 0; y < Max_y; ++y )
                if( Linked<ToAbove>( x, y ) ||
                    Linked<ToRight>( x, y ) )
                    return false;
        return Result;
    }

    auto Isolated( auto x, auto y ) const
    {
        if( x >= Max_x || y >= Max_y || Empty( x, y ) ) return true;

        if( Linked<ToAbove>( x, y ) ||
            Linked<ToBelow>( x, y ) ||
            Linked<ToRight>( x, y ) ||
            Linked<ToLeft>( x, y ) ) return false;
        return true;
    }

    auto Validate( auto Input ) const
    {
        auto NextMove = Input();
        while( Isolated( NextMove.x, NextMove.y ) )
            NextMove = Input();
        return NextMove;
    }

    auto FloodFill( auto x, auto y ) -> void
    {
        const auto TargetColour = std::exchange( At( x, y ), Colour::EMPTY );

        if( Matched<ToAbove>( x, y, TargetColour ) ) FloodFill( x, y + 1 );
        if( Matched<ToBelow>( x, y, TargetColour ) ) FloodFill( x, y - 1 );
        if( Matched<ToRight>( x, y, TargetColour ) ) FloodFill( x + 1, y );
        if( Matched<ToLeft>( x, y, TargetColour ) ) FloodFill( x - 1, y );
    }

    auto ColumnShrink()
    {
        for( auto x = 0; x < Max_x; ++x )
            for( auto y = 0, new_y = 1; y < Max_y; ++y, ++new_y )
                if( Empty( x, y ) )
                {
                    while( new_y < Max_y && Empty( x, new_y ) ) ++new_y;
                    if( new_y < Max_y )
                        At( x, y ) = std::exchange( At( x, new_y ), Colour::EMPTY );
                    else
                        y = Max_y;
                }
    }

    auto RowShrink()
    {
        for( auto x = 0, new_x = 1; x < Max_x; ++x, ++new_x )
            if( Empty( x, 0 ) )
            {
                while( new_x < Max_x && Empty( new_x, 0 ) ) ++new_x;
                if( new_x < Max_x )
                    At( x ) = std::exchange( At( new_x ), Column{ Colour::EMPTY } );
                else
                    x = Max_x;
            }
    }

    auto StrikeOut( auto x, auto y )
    {
        FloodFill( x, y );
        ColumnShrink();
        RowShrink();
    }

    auto ColouredDisplay() const
    {
        for( auto y = ColumnSize; y-- > 0; )
        {
            cout << y << "| ";
            for( auto x = 0; x < RowSize; ++x )
                cout << ColourBlock( std::to_underlying( At( x, y ) ) );
            cout << '\n';
        }

        cout << "    _ _ _ _ _ _ _ _ _ _\n   ";
        for( auto x = 0; x < RowSize; ++x ) cout << " " << x;

        cout << '\n'<<std::endl;
    }

    auto MainLoop()
    {
        while( ! Exhausted() )
        {
            ColouredDisplay();
            auto [ x, y ] = Validate( RandomInput );
            //auto [ x, y ] = Validate( UserInput );
            StrikeOut( x, y );
        }
        ColouredDisplay();
        return 1;
    }
};

static auto disable_stdio_sync = [] {
    return std::ios_base::sync_with_stdio( false );
}();

int main()
{
    return Board{}.MainLoop();
}