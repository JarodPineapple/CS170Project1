#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>

using namespace std;
vector<int> goal = {1, 2, 3, 4, 5, 6, 7, 8, 0}; // Perfect Puzzle

// Struct that represents the puzzle's state and data
struct PuzzleState
{
    vector<int> state; // Vector of integers of the current puzzle state
    double heuristic; // Heuristic cost. Can be set to 0 for UCS
    int depth; // Cost to reach the state
    int zero; // Location of the zero
};

// Prints out a given puzzle
void Print(const vector<int>& state)
{
    for (int i = 0; i < state.size(); i++)
    {
        if (state[i] == 0)
        {
            cout << "b ";
        }
        else
        {
            cout << state[i] << " ";
        }

        if ((i + 1) % 3 == 0)
        {
            cout << endl;
        }
    }
}

// This function creates all possible next moves from the current puzzle state
// Simulates moving the blank tile left, right, up, and down
// No invalid moves are allowed
// Returns a vector of PuzzleStates representing new valid states
vector<PuzzleState> ValidMoveset(const PuzzleState& curr)
{
    vector<PuzzleState> nextMoves; // Vector of PuzzleStates to store next states
    vector<int> moves = {-1, 1, -3, 3}; // Left, right, up, down
    int blankTile = curr.zero; // Current blank tile position

    // Iterate over each possible direction
    for (int mov : moves)
    {
        int newPos = blankTile + mov; // Get the new position of the blank tile

        // Check and prevent any invalid moves
        // Prevents wrapping around the left edge to the right and from right to left
        if (newPos < 0 || newPos >= 9 || (blankTile % 3 == 0 && mov == -1) || (blankTile % 3 == 2 && mov == 1))
        {
            continue;
        }

        // New state by swaping blank tile with new position
        vector<int> newPState = curr.state;
        swap(newPState[blankTile], newPState[newPos]);
        // Add state to the nextMoves list
        nextMoves.push_back({newPState, 0, curr.depth + 1, newPos});
    }

    return nextMoves; // Function returns all valid next moves
}

// This function calculates the Euclidean distance heurisitic for a puzzle state
// Estimates the cost to get to the perfect puzzle
// Straight distance between current position of a tile and its target position in the perfect puzzle
double Euclidean(const vector<int>& state)
{
    double dis = 0; // Distance starts at 0

    // Iterate through each tile in the state
    for (int i = 0; i < state.size(); i++)
    {
        // Ignore the blank tile
        if (state[i] == 0)
        {
            continue;
        }

        // Calculate the row and column for the tile
        int targetRow = (state[i] - 1) / 3;
        int targetCol = (state[i] - 1) % 3;
        // Calculate the current row and column for the tile
        int currRow = i / 3;
        int currCol = i % 3;
        // Add the Euclidean distance to the total distance
        dis = dis + sqrt(pow(targetRow - currRow, 2) + pow(targetCol - currCol, 2));
    }

    return dis; // Function returns a double that represents the total Euclidean distance
}

// This function adds up the number of out of place tiles compared to the perfect puzzle
// Blank tile is excluded
// Returns the total number as an int
int OutOfPlace(const vector<int>& state)
{
    int outofplace = 0;

    // Iterate through each tile in the state
    for (int i = 0; i < state.size(); i++)
    {
        // Check if the tile is out of place and not blank
        if (state[i] != 0 && state[i] != goal[i])
        {
            outofplace++; // Increment
        }
    }

    return outofplace; // Returns an int with the number of tiles out of place
}

// This function forms the Uniform Cost Search algorithm to solve the puzzle
// It expands the least-cost state first, and does not look at heuristics
// Outputs statistics and stops when the goal is found, without reconstructing the solution path
void UniformCostSearch(vector<int>& puzzle)
{
    vector<PuzzleState> open; // States to explore
    vector<vector<int>> explored; // Explored states
    int expanded = 0; // States expanded
    int maxSize = 0; // Max size of open

    // Start state
    PuzzleState startState = {puzzle, 0, 0, (int)(find(puzzle.begin(), puzzle.end(), 0) - puzzle.begin())};
    open.push_back(startState);

    cout << "Expanding state:" << endl;
    Print(puzzle);
    cout << endl;

    // While we still have states to explore
    while (!open.empty())
    {
        maxSize = max(maxSize, (int)open.size()); // Update size
        
        // Search for index of the state with the lowest cost
        int mindex = 0; // Initialize the index with the minimum element
        for (int i = 1; i< open.size(); i++)
        {
            if (open[i].depth < open[mindex].depth)
            {
                mindex = i; // If a lower cost is found, update our index
            }
        }

        // Select and remove the ideal state
        PuzzleState curr = open[mindex];
        open.erase(open.begin() + mindex);

        cout << "The best state to expand with g(n) = " << curr.depth << " and h(n) = 0 is..." << endl;
        Print(curr.state);
        cout << endl << "Expanding this node..." << endl << endl;

        // Check for a winning state
        if (curr.state == goal)
        {
            cout << "Goal!!!" << endl;
            cout << "To solve this problem the search algorithm expanded a total of " << expanded << " nodes." << endl;
            cout << "The maximum number of nodes in the queue at any one time: " << maxSize << endl;
            cout << "The depth of the goal node was : " << curr.depth << endl;
            return;
        }
        
        // Add state to explored list
        explored.push_back(curr.state);
        // Increment the state counter
        expanded++;

        // Create heirs
        vector<PuzzleState> heirs = ValidMoveset(curr);
        for (PuzzleState& heir : heirs)
        {
            bool previouslyExplored = false;
            for (const vector<int> &exploredState : explored)
            {
                if (exploredState == heir.state)
                {
                    previouslyExplored = true;
                    break;
                }
            }

            if (!previouslyExplored)
            {
                open.push_back(heir);
            }
        }
    }

    cout << "Uniform Cost Search could not find a solution." << endl;
    cout << "States expanded: " << expanded << endl;
    cout << "Max queue size: " << maxSize << endl;
}

// This function is the A* search algorithm using the misplaced tile heuristic
// Prioritizes states with lowest combined cost to explore the most promising path first
// Outputs statistics and stops when the goal is found, without reconstructing the solution path
void AStarMisplaced(vector<int>& puzzle)
{
    vector<PuzzleState> open; // States to explore
    vector<vector<int>> explored; // Explored states
    int expanded = 0; // States expanded
    int maxSize = 0; // Max size of open

    // Starting state assigned with heuristic value
    PuzzleState start = {puzzle, 0, OutOfPlace(puzzle), (int)(find(puzzle.begin(), puzzle.end(), 0) - puzzle.begin())};
    // Add the starting state to the states to explore list
    open.push_back(start);

    cout << "Expanding state:" << endl;
    Print(puzzle);
    cout << endl;

    // While we still have states to explore
    while (!open.empty())
    {
        maxSize = max(maxSize, (int)open.size());

        // Search for index of the state with the lowest cost
        int mindex = 0;
        for (int i = 1; i < open.size(); i++)
        {
            if ((open[i].depth + open[i].heuristic) < (open[mindex].depth + open[mindex].heuristic))
            {
                mindex = i;
            }
        }

        // Select and remove the ideal state
        PuzzleState curr = open[mindex];
        open.erase(open.begin() + mindex);

        curr.heuristic = OutOfPlace(curr.state);
        cout << "The best state to expand with g(n) = " << curr.depth << " and h(n) = " << curr.heuristic << " is..." << endl;
        Print(curr.state);
        cout << endl << "Expanding this node..." << endl << endl;

        // Check for a winning state
        if (curr.state == goal)
        {
            cout << "Goal!!!" << endl;
            cout << "To solve this problem the search algorithm expanded a total of " << expanded << " nodes." << endl;
            cout << "The maximum number of nodes in the queue at any one time: " << maxSize << endl;
            cout << "The depth of the goal node was : " << curr.depth << endl;
            return;
        }

        // Add state to explored list
        explored.push_back(curr.state);
        // Increment the state counter
        expanded++;

        // Create heirs
        vector<PuzzleState> heirs = ValidMoveset(curr);
        for (PuzzleState& heir : heirs)
        {
            heir.heuristic = OutOfPlace(heir.state);
            bool previouslyExplored = false;
            for (const vector<int> &exploredState : explored)
            {
                if (exploredState == heir.state)
                {
                    previouslyExplored = true;
                    break;
                }
            }

            // Check if the state is already in the list
            bool inside = false;
            for (const PuzzleState& openState : open)
            {
                if (openState.state == heir.state)
                {
                    inside = true;
                    break;
                }
            }

            if (!previouslyExplored && !inside)
            {
                open.push_back(heir);
            }
        }
    }

    cout << "A* with the Misplaced Tile heuristic could not find a solution." << endl;
    cout << "States expanded: " << expanded << endl;
    cout << "Max queue size: " << maxSize << endl;
}

// This function is the A* search algorithm using Euclidean distance heuristic
// Calculates the straight-line distance for each tile from its current position to the goal
// Outputs statistics and stops when the goal is found, without reconstructing the solution path
void AStarEuclidean(vector<int>& puzzle)
{
    vector<PuzzleState> open; // States to explore
    vector<vector<int>> explored; // Explored states
    int expanded = 0; // States expanded
    int maxSize = 0; // Max size of open

    // Starting state assigned with heuristic value
    PuzzleState start = {puzzle, 0, Euclidean(puzzle), static_cast<int>(find(puzzle.begin(), puzzle.end(), 0) - puzzle.begin())};
    // Add the starting state to the states to explore list
    open.push_back(start);

    cout << "Expanding state" << endl;
    Print(puzzle);
    cout << endl;

    // While we still have states to explore
    while (!open.empty())
    {
        maxSize = max(maxSize, (int)open.size());

        // Search for index of the state with the lowest cost
        int mindex = 0;
        for (int i = 1; i < open.size(); i++)
        {
            if ((open[i].depth + open[i].heuristic) < (open[mindex].depth + open[mindex].heuristic))
            {
                mindex = i;
            }
        }

        // Select and remove the ideal state
        PuzzleState curr = open[mindex];
        open.erase(open.begin() + mindex);

        curr.heuristic = Euclidean(curr.state);
        cout << "The best state to expand with g(n) = " << curr.depth << " and h(n) = " << curr.heuristic << " is..." << endl;
        Print(curr.state);
        cout << endl << "Expanding this node..." << endl << endl;

        // Check for a winning state
        if (curr.state == goal)
        {
            cout << "Goal!!!" << endl;
            cout << "To solve this problem the search algorithm expanded a total of " << expanded << " nodes." << endl;
            cout << "The maximum number of nodes in the queue at any one time: " << maxSize << endl;
            cout << "The depth of the goal node was : " << curr.depth << endl;
            return;
        }

        // Add state to explored list
        explored.push_back(curr.state);
        // Increment the state counter
        expanded++;

        // Create heirs
        vector<PuzzleState> heirs = ValidMoveset(curr);
        for (PuzzleState& heir : heirs)
        {
            heir.heuristic = Euclidean(heir.state);
            bool previouslyExplored = false;
            for (const vector<int> &exploredState : explored)
            {
                if (exploredState == heir.state)
                {
                    previouslyExplored = true;
                    break;
                }
            }

            // Check if the state is already in the list
            bool inside = false;
            for (const PuzzleState& openState : open)
            {
                if (openState.state == heir.state)
                {
                    inside = true;
                    break;
                }
            }

            if (!previouslyExplored && !inside)
            {
                open.push_back(heir);
            }
        }
    }

    cout << "A* with the Euclidean Distance heuristic could not find a solution." << endl;
    cout << "States expanded: " << expanded << endl;
    cout << "Max queue size: " << maxSize << endl;
}

void CreatePuzzle(int choicePuzz, vector<int>& puzzle)
{
    int flag = 1;
    int userVal = 0;
    vector<int> expected ={0, 1, 2, 3, 4, 5, 6, 7, 8};
    vector<int> check;

    if (choicePuzz == 1)
    {
        puzzle = {1, 2, 3, 4, 8, 0, 7, 6, 5};
    }
    else if (choicePuzz == 2)
    {
        while (flag == 1)
        {
            cout << "Enter your puzzle, use a zero to represent the blank." << endl;
            cout << "Enter the first row, use a space or tabs between numbers: ";
            cin >> userVal;
            puzzle.push_back(userVal);
            cin >> userVal;
            puzzle.push_back(userVal);
            cin >> userVal;
            puzzle.push_back(userVal);
            cout << "Enter the second row, use space or tabs between numbers: ";
            cin >> userVal;
            puzzle.push_back(userVal);
            cin >> userVal;
            puzzle.push_back(userVal);
            cin >> userVal;
            puzzle.push_back(userVal);
            cout << "Enter the third row, use space or tabs between numbers: ";
            cin >> userVal;
            puzzle.push_back(userVal);
            cin >> userVal;
            puzzle.push_back(userVal);
            cin >> userVal;
            puzzle.push_back(userVal);

            check = puzzle;
            sort(check.begin(), check.end());

            if (check != expected)
            {
                cout << "Puzzle needs to contain 0-8 only, each number once." << endl;
                puzzle.clear();
            }
            else
            {
                flag = 0;
            }
        }
    }
    else
    {
        cout << "An error occurred. Puzzle choice does not register as 1 or 2." << endl;
    }
}

// Checks if the given puzzle has a possible solution
// If the created puzzle has an even amount of inversions, it can be solved
// Returns true if the inversions are even
bool Possible(const vector<int>& puzzle)
{
    int inversions = 0;
    for (int i = 0; i < puzzle.size(); i++)
    {
        // Don't include the blank space
        if (puzzle[i] == 0)
        {
            continue;
        }

        for (int j = i + 1; j < puzzle.size(); j++)
        {
            // Don't include the blank space
            if (puzzle[j] == 0)
            {
                continue;
            }
            if (puzzle[i] > puzzle[j])
            {
                inversions++;
            }
        }
    }

    return inversions % 2 == 0;
}

int main()
{
    int flag = 1;
    int choicePuzz = 0;
    int choiceAlgo = 0;
    vector<int> puzzle;

    cout << "Welcome to Jarod Hendrickson's 8 puzzle solver." << endl;
    cout << "Type 1 to use a default puzzle, or 2 to enter your own puzzle." << endl << endl;
    cout << "Your choice is: ";

    while (flag == 1)
    {
        cin >> choicePuzz;

        if (choicePuzz == 1)
        {
            flag = 0;
            cout << endl;
        }
        else if (choicePuzz == 2)
        {
            flag = 0;
            cout << endl;
        }
        else
        {
            cout << "Not a valid choice, try again." << endl;
            cout << "Your choice is: ";
        }
    }

    flag = 1;

    cout << "Enter your choice of algorithm:" << endl;
    cout << "1. Uniform Cost Search" << endl;
    cout << "2. A* with the Misplaced Tile heuristic" << endl;
    cout << "3. A* with the Euclidean distance heuristic" << endl << endl;
    cout << "Your choice is: ";

    while (flag == 1)
    {
        cin >> choiceAlgo;

        if (choiceAlgo == 1)
        {
            flag = 0;
            cout << endl;
        }
        else if (choiceAlgo == 2)
        {
            flag = 0;
            cout << endl;
        }
        else if (choiceAlgo == 3)
        {
            flag = 0;
            cout << endl;
        }
        else
        {
            cout << "Not a valid choice, try again." << endl;
            cout << "Your choice is: ";
        }
    }

    cout << "You picked " << choicePuzz << " and " << choiceAlgo << endl << endl;

    CreatePuzzle(choicePuzz, puzzle);

    cout << "Puzzle was created successfully." << endl;

    if (!Possible(puzzle))
    {
        cout << "This puzzle is not able to be solved. Try a new combination." << endl;
        return 0;
    }

    if (choiceAlgo == 1)
    {
        UniformCostSearch(puzzle);
    }
    else if (choiceAlgo == 2)
    {
        AStarMisplaced(puzzle);
    }
    else if (choiceAlgo == 3)
    {
        AStarEuclidean(puzzle);
    }
    else
    {
        cout << "An error occurred. Algorithm choice does not register as 1, 2, or 3." << endl;
    }

    return 0;
}