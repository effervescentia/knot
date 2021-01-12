include Stdlib.String;

module TString = Tablecloth.String;

let of_uchars = (cs: list(Uchar.t)) =>
  cs
  |> List.fold_left(
       (acc, r) => {
         Buffer.add_utf_8_uchar(acc, r);
         acc;
       },
       Buffer.create(List.length(cs)),
     )
  |> Buffer.contents;

let join = (~separator="", xs: list(string)): string =>
  TString.join(~sep=separator, xs);

let starts_with = (prefix: string, value: string): bool =>
  TString.starts_with(~prefix, value);

let ends_with = (suffix: string, value: string): bool =>
  TString.ends_with(~suffix, value);

let drop_left = (count: int, value: string): string =>
  TString.drop_left(~count, value);

let drop_right = (count: int, value: string): string =>
  TString.drop_right(~count, value);

let drop_prefix = (prefix: string, value: string): string =>
  starts_with(prefix, value) ? drop_left(length(prefix), value) : value;

let drop_suffix = (suffix: string, value: string): string =>
  ends_with(suffix, value) ? drop_right(length(suffix), value) : value;

let repeat = (count: int, value: string) => TString.repeat(~count, value);