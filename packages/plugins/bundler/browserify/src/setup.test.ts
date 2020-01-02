// tslint:disable:no-expression-statement no-object-literal-type-assertion
import test from 'ava';
import { BrowserifyObject } from 'browserify';
import { addExtension } from './setup';

test('addExtension(): add extensions', t => {
  const bundle = { _extensions: ['.js'] } as BrowserifyObject;

  addExtension(bundle);

  t.deepEqual(bundle._extensions, ['.kn', '.knot', '.js']);
});

test('addExtension(): skip adding existing extension', t => {
  const bundle = { _extensions: ['.js', '.kn'] } as BrowserifyObject;

  addExtension(bundle);

  t.deepEqual(bundle._extensions, ['.knot', '.js', '.kn']);
});
