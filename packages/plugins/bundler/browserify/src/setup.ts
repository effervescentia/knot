import { FILE_EXTENSIONS, Options, resolveLibrary } from '@knot/compiler';
import { BrowserifyObject } from 'browserify';

export function addExtension(bundle: BrowserifyObject): void {
  FILE_EXTENSIONS.forEach(extension => {
    if (!bundle._extensions.includes(extension)) {
      bundle._extensions.unshift(extension);
    }
  });
}

function extendModuleResolution(
  bundle: BrowserifyObject,
  options: Options
): void {
  const originalResolve = bundle._mdeps.resolve.bind(bundle._mdeps);

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
