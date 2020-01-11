import { StateFactory } from '@knot/plugin-utils';

class VueStateFactory<S extends object> implements StateFactory<S> {
  constructor(private state: S) {}

  prop<K extends keyof S>(name: K, property: S[K]) {
    return (this.state[name] = property);
  }

  mut<K extends keyof S, T extends S[K] & ((value: any) => void)>(
    name: K,
    mutator: T
  ) {
    return (this.state[name] = mutator);
  }

  build() {
    return this.state;
  }
}

export default VueStateFactory;
