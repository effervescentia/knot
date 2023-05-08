import { createEnumerated, EnumInstanceType } from '@knot/common';

// length

const LENGTH_VARIANTS = [
  'Ch',
  'Em',
  'Ex',
  'Ic',
  'Rem',

  'Vh',
  'Vw',
  'VMax',
  'VMin',
  'Vb',
  'Vi',

  'Px',
  'Cm',
  'Mm',
  'Q',
  'In',
  'Pc',
  'Pt',
] as const;

export const Length =
  createEnumerated<Record<typeof LENGTH_VARIANTS[number], [number]>>(
    LENGTH_VARIANTS
  );

const lengthToString = ([{ name }, length]: [
  EnumInstanceType<typeof Length>,
  number
]): string => `${length}${name.toLocaleLowerCase()}`;

// color

export const Color = createEnumerated<{ Named: [string] }>(['Named']);

const colorToString = ([variant, ...args]: ReturnType<
  typeof Color[keyof typeof Color]
>): string => {
  // eslint-disable-next-line sonarjs/no-small-switch
  switch (variant as any) {
    case Color.Named:
      return args[0];
    default:
      return '';
  }
};

// rules

export const height = lengthToString;
export const width = lengthToString;

// color rules

export const color = colorToString;
export const backgroundColor = colorToString;
export const borderColor = colorToString;
export const outlineColor = colorToString;
export const textDecorationColor = colorToString;
export const textEmphasisColor = colorToString;
export const textShadow = colorToString;
export const caretColor = colorToString;
export const columnRuleColor = colorToString;
export const printColorAdjust = colorToString;
