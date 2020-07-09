import 'browserify';

import { ModuleDepsObject } from 'module-deps';

declare module 'browserify' {
  interface BrowserifyObject {
    /** accepted extensions */
    _extensions: string[];
    _mdeps: ModuleDepsObject;

    on(
      event: 'update',
      callback: (contents: Buffer, changedFiles: string[]) => void
    ): this;
  }
}
