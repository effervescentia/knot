import { Brand } from 'utility-types';

export type EnumInstance<Instance, Enum extends Record<string, any[]>> = {
  [K in keyof Enum]: (...args: Enum[K]) => [Instance, ...Enum[K]];
};

export type EnumInstanceType<Enum> = Enum extends EnumInstance<
  infer R,
  Record<string, any>
>
  ? R
  : never;

export const createEnumerated = <
  Enum extends Record<string, any[]>,
  Instance = Brand<{ name: string }, Enum>
>(
  variants: readonly (keyof Enum)[]
) =>
  Object.fromEntries(
    variants.map((name) => {
      const factory = (...args: unknown[]) => [factory, ...args];

      Object.defineProperty(factory, 'name', {
        value: name,
        configurable: true,
      });

      return [name, factory];
    })
  ) as EnumInstance<Instance, Enum>;
