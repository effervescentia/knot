import { Options } from 'nps-utils';

import build from './build';
import cov from './cov';
import doc from './doc';
import fix from './fix';
import test from './test';
import * as Utility from './utility';

export const DEFAULT_OPTIONS: Options = {
  helpStyle: 'scripts'
};

export const DEFAULT_SCRIPTS = {
  ...Utility,

  build,
  fix,
  test,
  cov,
  doc
};

export function extendScripts(scripts: object = {}): object {
  return {
    options: DEFAULT_OPTIONS,

    scripts: {
      ...DEFAULT_SCRIPTS,

      ...scripts
    }
  };
}

export default extendScripts();
