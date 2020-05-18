import { getOptions } from 'loader-utils';

import log from './logger';
import { InternalOptions } from './types';

export = function loader(): void {
  const callback = this.async();
  const options = getOptions(this) as InternalOptions;
  const path = this._module.request;

  options.compilerInstance
    .awaitModule(path)
    .then(() => options.compilerInstance.generate(path))
    .then(result => {
      log.error('%s: %s', path, result);

      return callback(null, result);
    })
    .catch(callback);
};
