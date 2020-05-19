import { fromEntries } from '@knot/common';
import { StateFactory } from '@knot/plugin-utils';
import Vue from 'vue';

class VueStateFactory<S extends object> implements StateFactory<S> {
  private properties: (keyof S)[] = [];
  private mutators: (keyof S)[] = [];

  constructor(private forceUpdate: () => void) {}

  prop<K extends keyof S>(name: K, property: S[K]) {
    this.properties.push(name);

    return property;
  }

  mut<K extends keyof S, T extends S[K] & ((value: any) => void)>(
    name: K,
    mutator: T
  ) {
    const forceUpdate = this.forceUpdate;

    this.mutators.push(name);

    return function() {
      // eslint-disable-next-line prefer-rest-params, prefer-spread
      mutator.apply(null, arguments);
      forceUpdate();
    } as any;
  }

  build(state: S) {
    const observed = Vue.observable(
      fromEntries<S>(this.properties.map(name => [name, state[name]]))
    );

    const result = {} as S;

    Object.defineProperties(
      result,
      fromEntries<PropertyDescriptorMap>(
        this.properties.map(name => [
          name as string,
          {
            configurable: false,
            enumerable: true,
            get() {
              return observed[name];
            }
          }
        ])
      )
    );

    this.mutators.forEach(name => {
      result[name] = state[name];
    });

    return result;
  }
}

export default VueStateFactory;
