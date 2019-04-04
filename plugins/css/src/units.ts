// tslint:disable:object-literal-sort-keys
export default {
  ch: unit('ch'),
  cm: unit('cm'),
  deg: unit('deg'),
  dpcm: unit('dpcm'),
  dpi: unit('dpi'),
  dppx: unit('dppx'),
  em: unit('em'),
  ex: unit('ex'),
  fr: unit('fr'),
  Hz: unit('Hz'),
  in: unit('in'),
  kHz: unit('kHz'),
  mm: unit('mm'),
  ms: unit('ms'),
  pc: unit('pc'),
  pt: unit('pt'),
  px: unit('px'),
  rem: unit('rem'),
  s: unit('s'),
  vh: unit('vh'),
  vw: unit('vw'),
  x: unit('x')
};

export function unit(value: string): (num: number) => string {
  return (num: number) => String(num) + value;
}
