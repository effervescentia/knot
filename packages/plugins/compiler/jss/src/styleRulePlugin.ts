import { createEnumerated, EnumInstanceType } from '@knot/common';

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

export const height = lengthToString;
export const width = lengthToString;
