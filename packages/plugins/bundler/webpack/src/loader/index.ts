// tslint:disable:no-expression-statement
import { getOptions } from 'loader-utils';
import { InternalOptions } from '../types';
import nodeTransformer from './node';

export = function loader(): void {
  const callback = this.async();
  const options = getOptions(this) as InternalOptions;
  const path = this._module.request;

  options.compilerInstance
    .awaitModule(path)
    .then(() => options.compilerInstance.generate(path))
    .then((result: string) => nodeTransformer(result, options))
    .then(result => callback(null, result))
    .catch(callback);
};
