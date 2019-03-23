// tslint:disable:no-expression-statement no-object-literal-type-assertion
import test from 'ava';
import * as path from 'path';
// tslint:disable-next-line:no-implicit-dependencies
import * as Webpack from 'webpack';
import KnotCompilerPlugin, {
  addModuleLoader,
  DEFAULT_OPTIONS,
  discoverDependencies,
  invalidateModule
} from '.';
import { Compiler, Options } from './types';

import WebpackModule = Webpack.loader.LoaderContext;

test('construct', t => {
  const options = { port: 90210 } as Options;

  const plugin = new KnotCompilerPlugin(options);

  t.deepEqual(plugin.options, { ...DEFAULT_OPTIONS, ...options });
});

test('invalidateModule() - path does not match regex', t => {
  const onInvalidate = invalidateModule(null, null);

  t.false(onInvalidate('myPath'));
});

test('invalidateModule() - path does match regex', async t => {
  const filePath = 'myPath.kn';
  const compiler = {
    invalidate: target => {
      t.is(target, filePath);

      return Promise.resolve();
    }
  } as Compiler;
  const kill = () => null;

  const onInvalidate = invalidateModule(compiler, kill);
  await onInvalidate(filePath);
});

test('invalidateModule() - invalidation fails', async t => {
  t.plan(1);
  const compiler = {
    invalidate: _ => Promise.reject()
  } as Compiler;
  const kill = () => Promise.resolve(t.pass());

  const onInvalidate = invalidateModule(compiler, kill);
  await onInvalidate('myPath.kn');
});

test('addModuleLoader() - path does not match regex', t => {
  const loaders: ReadonlyArray<any> = [];

  addModuleLoader(null)(null, {
    loaders,
    request: 'myModule'
  } as WebpackModule);

  t.deepEqual(loaders, []);
});

test('addModuleLoader() - path does match regex', t => {
  const loader = { loader: 'my-loader' } as Webpack.Loader;
  const loaders: ReadonlyArray<any> = [];

  addModuleLoader(loader)(null, {
    loaders,
    request: 'myModule.kn'
  } as WebpackModule);

  t.is(loaders.length, 1);
  t.is(loaders[0], loader);
});

test('addModuleLoader() - called with the same module multiple times', t => {
  const loader = { loader: 'my-loader' } as Webpack.Loader;
  const loaders: ReadonlyArray<any> = [];
  const mod = { loaders, request: 'myModule.kn' } as WebpackModule;

  addModuleLoader(loader)(null, mod);
  addModuleLoader(loader)(null, mod);

  t.is(loaders.length, 1);
});

test('discoverDependencies() - no matching dependencies', async t => {
  t.plan(0);

  const compiler = { add: _ => t.fail() } as Compiler;

  const onDiscover = discoverDependencies(compiler, null);

  await onDiscover({ dependencies: [] });
  await onDiscover({
    dependencies: [
      { request: 'some-module.js' },
      { request: 'other-module.ts' }
    ]
  });
});

test('discoverDependencies() - some matching dependencies', async t => {
  t.plan(2);

  const sourceDir = 'source/code';
  const dep1 = { request: 'knot-module.kn' };
  const dep2 = { request: 'app-module.knot' };
  const matchesRequest = (target: string, dep) =>
    t.is(target, path.resolve(__dirname, `../../${sourceDir}/${dep.request}`));

  // tslint:disable-next-line:no-let
  let attempts = 0;
  const compiler = {
    add: async target => {
      attempts++;

      switch (attempts) {
        case 1:
          return matchesRequest(target, dep1);
        case 2:
          return matchesRequest(target, dep2);
        default:
          return t.fail();
      }
    }
  } as Compiler;

  const onDiscover = discoverDependencies(compiler, null);
  await onDiscover({
    dependencies: [
      dep1,
      { request: 'some-module.js' },
      { request: 'other-module.ts' },
      dep2
    ],
    resource: `${sourceDir}/entry.kn`
  });
});
