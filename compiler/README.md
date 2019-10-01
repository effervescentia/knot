# knot

**Contains the following libraries and executables:**

```
knot@0.0.0
│
├─test/
│   name:    TestKnot.exe
│   main:    TestKnot
│   require: knot.lib, knot.lex, knot.parse, knot.analyze, knot.generate
│
├─library/
│   library name: knot.lib
│   namespace:    Knot
│   require:
│
├─lexer/
│   library name: knot.lex
│   namespace:    KnotLex
│   require:      knot.lib
│
├─parser/
│   library name: knot.parse
│   namespace:    KnotParse
│   require:      knot.lib
│
├─resolver/
│   library name: knot.resolve
│   namespace:    KnotResolve
│   require:      knot.lib
│
├─analyzer/
│   library name: knot.analyze
│   namespace:    KnotAnalayze
│   require:      knot.lib, knot.resolve
│
├─generator/
│   library name: knot.generate
│   namespace:    KnotGenerate
│   require:      knot.lib
│
├─compiler/
│   library name: knot.compile
│   namespace:    KnotCompile
│   require:      knot.lib, knot.lex, knot.parse, knot.analyze
│
└─executable/
    name:    knotc.exe
    main:    Main
    require: knot.lib, knot.generate, knot.compile
```

## Developing:

```
brew install opam
opam init
opam install reason

npm install -g esy

git clone <this-repo>
esy install
esy build
```

## Running Binary:

After building the project, you can run the main binary that is produced.

```
esy x knotc.exe
```

## Running Tests:

```
# Runs the "test" command in `package.json`.
esy test
```
