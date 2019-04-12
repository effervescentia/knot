// tslint:disable:no-expression-statement
import { getOptions } from 'loader-utils';
import { InternalOptions } from '../types';
import nodeResolver from './node';

export = function loader(): void {
  const callback = this.async();
  const options = getOptions(this) as InternalOptions;
  const path = this._module.request;

  options.compiler
    .awaitModule(path)
    .then(() => options.compiler.generate(path))
    .then((result: string) => nodeResolver(result, options))
    .then(result => callback(null, result))
    .catch(callback);
};
