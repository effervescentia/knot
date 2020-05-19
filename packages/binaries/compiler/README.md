# @knot/compiler

[![npm](https://img.shields.io/npm/v/@knot/compiler?style=flat-square)](http://npm.im/@knot/compiler)
[![code style: prettier](https://img.shields.io/badge/code_style-prettier-ff69b4.svg?style=flat-square)](https://github.com/prettier/prettier)
[![lerna](https://img.shields.io/badge/maintained%20with-lerna-cc00ff.svg?style=flat-square)](https://lerna.js.org/)
[![semantic-release](https://img.shields.io/badge/%20%20%F0%9F%93%A6%F0%9F%9A%80-semantic--release-e10079.svg?style=flat-square)](https://github.com/semantic-release/semantic-release)
[![Commitizen friendly](https://img.shields.io/badge/commitizen-friendly-brightgreen.svg?style=flat-square)](http://commitizen.github.io/cz-cli/)
[![standard-readme compliant](https://img.shields.io/badge/standard--readme-OK-green.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

[![CircleCI](https://img.shields.io/circleci/build/gh/effervescentia/knot?style=flat-square&token=c6d265c2c3ae9fea01043c75299974616b6498b0)](https://circleci.com/gh/effervescentia/knot)

> A command-line wrapper for the [`knot`](https://github.com/effervescentia/knot) compiler.
> The version of this package aligns with the version of the compiler binary that is downloaded.
> Exports the path to the downloaded `knotc` binary.
> Currently the compiler only supports Windows and MacOS.

The Knot Language compiler can be run as both:

- a stand-alone static compiler that scans a project directory structure and outputs
- a server which can dynamically re-compile modules within a project, but depends on a client to control it

To get started using it in a project you will probably want to use one of the provided bundler plugins:

- [`@knot/webpack-plugin`](http://npm.im/@knot/webpack-plugin): The official `knot` plugin for [`webpack`](https://webpack.js.org/)
- [`@knot/rollup-plugin`](http://npm.im/@knot/rollup-plugin): The official `knot` plugin for [`rollup`](https://rollupjs.org/)
- [`@knot/browserify-plugin`](http://npm.im/@knot/browserify-plugin): The official `knot` plugin for [`browserify`](http://browserify.org/)

Or you can use the `knot` CLI tool ([`@knot/cli`](http://npm.im/@knot/cli)) to generate a new project with the minimum required boilerplate.

The source code for the compiler that is wrapped by this package can be found [here](https://github.com/effervescentia/knot/tree/master/compiler);

## Table of Contents

- [Install](#install)
- [Usage](#usage)
- [Maintainers](#maintainers)
- [Contributing](#contributing)
- [License](#license)

## Install

```sh
npm install @knot/compiler
# or
yarn add @knot/compiler
```

## Usage

### Node.js Usage

#### path: _string_

The path to the installed binary.

```ts
import { path } from '@knot/compiler';

path; // path to the knotc binary
```

#### isKnot: _(fileName: string) => boolean_

Test whether a file has the file extension `.kn` or `.knot`.

```ts
import { isKnot } from '@knot/compiler';

isKnot('someFile.js'); // false
isKnot('knotFile.kn'); // true
isKnot('otherFile.knot'); // true
```

#### Compiler: _class KnotCompiler_

A client for connecting to and managing an instance of the `knot` compiler.

```ts
import Compiler from '@knot/compiler';

const compiler = new Compiler({});

// add a file to the compiler
await compiler.add('./someFile.kn');

// wait for the file to be compiled
await compiler.awaitModule('./someFile.kn');

// receive the comiled file contents
const compiled = await compiler.generate('./someFile.kn');

// kill the compiler server and end the connection
await compiler.close();
```

### CLI Usage

If you install the package globally, you should be able to invoke the `knotc` binary installed by this package.
If you install it locally, you can invoke it from a script in your `package.json` file, by running `yarn exec <cmd>`
or in a variety of other ways.

```sh
knotc -help
# or
yarn exec knotc -- -help
```

#### CLI Arguments

- `-server`: run the compiler in server mode
- `-config`: specify a path to the directory containing the `.knot.yml` file to load
- `-port`: specify a port for the compiler to run on, by default it runs on `:1338`
- `-debug`: print verbose logs
- `-help`: print usage information

## Maintainers

[@effervescentia](https://github.com/effervescentia)

## Contributing

PRs accepted.

Small note: If editing the README, please conform to the [standard-readme](https://github.com/RichardLitt/standard-readme) specification.

## License

MIT © 2019 Ben Teichman
