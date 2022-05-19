import KnotCompiler from '@knot/compiler';
import test from 'ava';
import * as path from 'path';
import * as Webpack from 'webpack';

import {
  addModuleLoader,
  discoverDependencies,
  invalidateModule
} from './utils';

import WebpackModule = Webpack.loader.LoaderContext;

test('invalidateModule() - path does not match regex', t => {
  const onInvalidate = invalidateModule(null, null);

  t.false(onInvalidate('myPath'));
});

test('invalidateModule() - path does match regex', async t => {
  const filePath = 'myPath.kn';
  const compiler = {
    update: target => {
      t.is(target, filePath);

      return Promise.resolve();
    }
  } as KnotCompiler;
  const kill = () => null;

  const onInvalidate = invalidateModule(compiler, kill);
  await onInvalidate(filePath);
});

test('invalidateModule() - invalidation fails', async t => {
  t.plan(1);
  const compiler = {
    update: _ => Promise.reject()
  } as KnotCompiler;
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

  const compiler = { add: _ => t.fail() } as KnotCompiler;

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
  } as KnotCompiler;

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
