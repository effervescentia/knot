export type ClassType<A extends any[] = any[], I extends {} = {}> = {
  new (...args: A): I;
};

export type FunctionType<A extends any[] = any[], R = any> = (...args: A) => R;

export type ObjectKeyType = string | number | symbol;

export type ObjectType<
  K extends ObjectKeyType = ObjectKeyType,
  V = any
> = Record<K, V>;
