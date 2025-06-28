# Onitama
This is an open-source simulation of the board game _[Onitama](https://www.arcanewonders.com/product/onitama/)_ by Shimpei Sato, written in C++ with a CLI.

This project contains a full implementation of the game's ruleset, with endpoints for strategy analysis and miscellaneous experimentation.

- [Features](#features)
- [Playing a game](#playing-a-game)
    - [Strategies](#strategies)
- [State graph construction](#state-graph-construction)
    - [Construction strategies](#construction-strategies)
- [Other commands](#other-commands)

## Features
- Onitama is fully playable by human players as well as hand-written AI strategies, with customization in the cards in play and the dimensions of the board.
- Strategies are abstracted into an extendable interface, making it easy to write custom AI strategies to be experimented on against other strategies.
- State graphs are fully implemented, with retrograde analysis and a custom forward-looking derivative thereof being able to compute perfect positional strategies, guaranteeing the mathematically optimal move to be played.

## Playing a game
A game can be played using the command
```
game <red_strategy> <blue_strategy> [options]
```
where the strategies provided can be any of the ones described in [Strategies](#strategies).

The options are as follows:
```
-d, --duplicate-cards   Allow for duplicate cards to be drawn.
-c, --cards             Provide a list of five cards to use, in the
                        order of _set-aside_, _red hand_, _blue hand_.
                        You can also provide a single card to be
                        duplicated five times.
                        If not supplied, the cards are chosen at
                        random.
-s, --size              Provide the width and height of the board, in
                        that order.
                        Width must be at least 1, and height must be
                        at least 2.
                        Default is 5x5.
-n, --repeat            Repeat this configuration the provided number
                        of times.
-p, --print-type        Defines how the game is to be printed.
                        - "board" prints the game board;
                        - "data" prints the sequence of moves
                          performed;
                        - "wins" prints the outcome of each game and
                          counts how many games were won by each
                          player;
                        - "none" doesn't print anything.
                        "board" is the default.
-m, --multithread       In conjunction with --repeat, plays all games
                        simultaneously on separate threads.
```

### Strategies
```
Human                   Play the game yourself through command-line input.
Random                  Choose a random valid move, uniformly distributed.
MonteCarlo <repeat_count>
                        Simulate <repeat_count> games for each valid move,
                        and pick the one that lead to the most wins.
MinMax [<max_depth>] [--no-max-depth]
                        Recursively simulate the game to find a perfect move
                        to play.
Positional [{-i | --import} <nodes-path> <edges-path>]
                        Pick the optimal move as dictated by a state graph.
                        If the optimal move is not yet defined, find it by
                        applying forward-looking retrograde analysis.
                        Optionally import a pre-computed state graph.
```

## State graph construction
State graphs can be constructed using the following subcommands:
```
experiment stategraph <construction_strategy> game [game_options] [general_options]
experiment stategraph <construction_strategy> state <state_id> [general_options]
```
`game_options` is the same as in [Playing a game](#playing-a-game).
`<construction_strategy>` must match one in [Construction strategies](#construction-strategies).
The general options are as follows:

```
-e, --export <nodes-path> <edges-path>
                        Export the constructed state graph after it is done.
-i, --import <nodes-path> <edges-path>
                        Import (part of) a state graph.
--images <images-path>  Export small images of each game state to the provided
                        directory.
--intermediate <file-path> <save-interval>
                        Save an intermediate state graph every <save-interval>
                        seconds.
--load <file-path>      Load an intermediate state graph.
--disable-symmetries    Count each game state as distinct, and do not apply
                        symmetries to cut down on the amount of game states
                        that need to be analysed.
--data                  Print the output data in csv format.
```

### Construction strategies
```
component <max_depth>   Map out all reachable game states from the initial
                        state. <max_depth> denotes the maximum recursion depth.
retrograde-analysis, retrograde <max_depth>
                        Construct a state graph using the `component strategy,
                        and analyse it using retrograde analysis.
forward-retrograde-analysis, forward-retrograde, forward
                        Construct a state graph and simultaneously analyse it
                        using forward-looking retrograde analysis.
                        Exits once it's found an ultra-weak solution.
dispersed-frontier, dispersed <depth> <max_thread_count>
                        Constructs the state graph in parallel, exploring up
                        to a depth of <depth> in each separate thread. At most
                        <max_thread_count> are active at one time.
```

## Other commands
```
help                    Print documentation.
version                 Print the current version of the software.
exit                    Exit the application.
cards                   Print all available cards.
strategies              Print all available strategies.
print game <game_id> [--image <image_path>]
                        Prints the provided game. Exports an image to the
                        provided path if `--image` is given.
```
