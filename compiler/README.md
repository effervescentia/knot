# tigris


[![CircleCI](https://circleci.com/gh/yourgithubhandle/tigris/tree/master.svg?style=svg)](https://circleci.com/gh/yourgithubhandle/tigris/tree/master)


**Contains the following libraries and executables:**

```
tigris@0.0.0
│
├─test/
│   name:    TestTigris.exe
│   main:    TestTigris
│   require: tigris.lib
│
├─library/
│   library name: tigris.lib
│   namespace:    Tigris
│   require:
│
└─executable/
    name:    TigrisApp.exe
    main:    TigrisApp
    require: tigris.lib
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
esy x TigrisApp.exe 
```

## Running Tests:

```
# Runs the "test" command in `package.json`.
esy test
```
