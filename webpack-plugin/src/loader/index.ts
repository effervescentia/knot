import { getOptions } from 'loader-utils';
import nodeResolver from './node';

export = function loader() {
  const callback = this.async();
  const options: any = getOptions(this);

  options.compiler
    .awaitComplete()
    .then(() => options.compiler.generate(this._module.request))
    .then(result => nodeResolver(result, options))
    .then(result => callback(null, result))
    .catch(callback);
};
