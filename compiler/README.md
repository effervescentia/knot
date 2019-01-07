# knot

[![CircleCI](https://circleci.com/gh/effervescentia/knot/tree/master.svg?style=svg)](https://circleci.com/gh/effervescentia/knot/tree/master)

**Contains the following libraries and executables:**

```
knot@0.0.0
│
├─test/
│   name:    TestKnot.exe
│   main:    TestKnot
│   require: knot.lib
│
├─library/
│   library name: knot.lib
│   namespace:    Knot
│   require:
│
└─executable/
    name:    KnotApp.exe
    main:    KnotApp
    require: knot.lib
```

## Developing:

```
npm install -g esy
git clone <this-repo>
esy install
esy build
```

## Running Binary:

After building the project, you can run the main binary that is produced.

```
esy x KnotApp.exe
```

## Running Tests:

```
# Runs the "test" command in `package.json`.
esy test
```
