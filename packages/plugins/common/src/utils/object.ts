import { ObjectKeyType } from '../types';

type Entry<T extends object> = [keyof T, T[keyof T]];

type GroupPredicateMap<T extends object> = Record<
  string,
  (entry: Entry<T>) => boolean
>;

export const addEntry = <T extends object>(
  acc: T,
  [key, value]: Entry<T>
): T => {
  acc[key] = value;

  return acc;
};

export const fromEntries = <T extends object>(
  entries: [keyof T, T[keyof T]][]
) => entries.reduce(addEntry, {} as T);

export const mapEntries = <T extends object, R extends object = T>(
  obj: T,
  map: (entries: Entry<T>[]) => Entry<R>[]
): R => {
  return fromEntries(map(Object.entries(obj) as any[]));
};

export const mapKeys = <
  T extends object,
  K extends ObjectKeyType = ObjectKeyType
>(
  obj: T,
  mapKey: (key: keyof T) => K
): Record<K, T[keyof T]> =>
  mapEntries<T, Record<K, T[keyof T]>>(obj, entries =>
    entries.map(([key, value]) => [mapKey(key), value])
  );

export const mapValues = <T extends object, R>(
  obj: T,
  mapValue: (value: T[keyof T]) => R
): Record<keyof T, R> =>
  mapEntries<T, Record<keyof T, R>>(obj, entries =>
    entries.map(([key, value]) => [key, mapValue(value)])
  );

export const filterEntries = <T extends object>(
  obj: T,
  filter: (entry: Entry<T>) => boolean
) => mapEntries(obj, entries => entries.filter(filter));

export const filterKeys = <T extends object>(
  obj: T,
  filter: (key: keyof T) => boolean
) => filterEntries(obj, ([key]) => filter(key));

export const filterValues = <T extends object>(
  obj: T,
  filter: (value: T[keyof T]) => boolean
) => filterEntries(obj, ([, value]) => filter(value));

export const groupReduce = <
  T extends object,
  G extends GroupPredicateMap<T>,
  R = Entry<T>
>(
  obj: T,
  groups: G,
  reduce: (acc: R, entry?: Entry<T>) => R,
  factory: () => R
): Record<keyof G, R> => {
  const keys: (keyof G)[] = Object.keys(groups);
  const group = fromEntries<Record<keyof G, R>>(
    keys.map(key => [key, factory()])
  );
  const entries = Object.entries(obj) as Entry<T>[];

  entries.forEach(entry => {
    for (const key of keys) {
      if (groups[key](entry)) {
        group[key] = reduce(group[key], entry);
        break;
      }
    }
  });

  return group;
};

export const groupValues = <T extends object, G extends GroupPredicateMap<T>>(
  obj: T,
  groups: G
): Record<keyof G, T[keyof T][]> =>
  groupReduce(
    obj,
    groups,
    (acc, [, value]) => {
      acc.push(value);

      return acc;
    },
    () => []
  );

export const groupEntries = <T extends object, G extends GroupPredicateMap<T>>(
  obj: T,
  groups: G
): Record<keyof G, Partial<T>> =>
  groupReduce(obj, groups, addEntry, () => ({} as any));
