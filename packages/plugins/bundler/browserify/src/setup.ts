// tslint:disable: no-expression-statement
import { Options, resolveLibrary } from '@knot/compiler';
import { BrowserifyObject } from 'browserify';

const FILE_EXTENSION = '.kn';

function addExtension(bundle: BrowserifyObject): void {
  if (!bundle._extensions.includes(FILE_EXTENSION)) {
    bundle._extensions.unshift(FILE_EXTENSION);
  }
}

function extendModuleResolution(
  bundle: BrowserifyObject,
  options: Options
): void {
  const originalResolve = bundle._mdeps.resolve.bind(bundle._mdeps);

  // tslint:disable-next-line: no-object-mutation
  bundle._mdeps.resolve = function knotResolve(id, parent, next): any {
    const resolved = resolveLibrary(id, options) || id;

    return originalResolve(resolved, parent, next);
  };
}

function setupPipeline(bundle: BrowserifyObject, options: Options): void {
  addExtension(bundle);
  extendModuleResolution(bundle, options);
}

export default setupPipeline;
