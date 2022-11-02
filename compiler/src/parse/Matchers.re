open Kore;
open Onyx;
open ModuleAliases;

module C = Constants;

let __back_slash = Uchar.of_char(C.Character.back_slash);

let space = one_of([' ', '\t', '\n']);
let spaces = skip_many(space);

let digit = range('0', '9');

let lower_alpha = range('a', 'z');
let upper_alpha = range('A', 'Z');
let alpha = lower_alpha <|> upper_alpha;

let alpha_num = digit <|> alpha;

let lexeme = x => spaces >> x;

let between = (l, r, x) =>
  map3((l', x', r') => N.untyped(x', N.join_ranges(l', r')), l, x, r);

let binary_op = (lx, op, rx) => map3((l, _, r) => (l, r), lx, op, rx);

let rec unary_op = (x, op) => op >>= (f => unary_op(x, op) >|= f) <|> x;

/**
 matches a single character
 */
let symbol = x => char(x) >|= Input.to_node |> lexeme;

/**
 matches a pattern that may be terminated by a semicolon
 */
let terminated = x => x << (C.Character.semicolon |> symbol |> optional);

/**
 matches a pattern that is separated by commas and may be terminated with a comma
 */
let comma_sep = x =>
  x
  |> sep_by(symbol(C.Character.comma))
  << optional(symbol(C.Character.comma));

/**
 matches an assignment operation
 */
let assign = (id, x) => binary_op(id, symbol(C.Character.equal_sign), x);

/**
 matches a sequence of characters but tolerates spaces in between
 */
let glyph = (s: string) =>
  get_cursor
  >>= (
    start => {
      let rec loop =
        fun
        | [] => assert(false)
        | [c] =>
          char(c)
          >|= Input.get_point
          >|= (end_ => N.untyped((), Range.create(start, end_)))
          |> lexeme
        | [c, ...cs] => char(c) |> lexeme >> loop(cs);

      s |> String.to_seq |> List.of_seq |> loop;
    }
  )
  |> lexeme;

/**
 matches a sequence of characters
 */
let keyword = (s: string) =>
  get_cursor
  >>= (
    start => {
      let rec loop =
        fun
        | [] => assert(false)
        | [c] =>
          char(c)
          >|= Input.get_point
          >|= (end_ => N.untyped(s, Range.create(start, end_)))
        | [c, ...cs] => char(c) >> loop(cs);

      loop(s |> String.to_seq |> List.of_seq)
      <<! (alpha_num <|> Character.underscore);
    }
  )
  |> lexeme;

/**
 matches a sequence of alpha-numeric characters
 */
let identifier = (~prefix=alpha <|> Character.underscore, input) =>
  (
    prefix
    <~> (alpha_num <|> Character.underscore |> many)
    >|= Input.join
    |> lexeme
  )(
    input,
  );

/**
 matches a sequence of characters between quotation marks

 allows escaping characters with a '\' character
 */
let string =
  Character.quote
  >|= Input.get_point
  >>= (
    start => {
      let rec loop = f =>
        choice([
          /* end of string sequence */
          Character.quote
          >|= Input.get_point
          >|= (
            end_ =>
              N.untyped(
                f([]) |> String.of_uchars,
                Range.create(start, end_),
              )
          ),
          /* capture escaped characters */
          Character.back_slash
          >> any
          >|= Input.get_value
          >>= (c => loop(rs => f([__back_slash, c, ...rs]))),
          /* capture characters of the string */
          none_of([C.Character.quote, C.Character.eol])
          >|= Input.get_value
          >>= (c => loop(rs => f([c, ...rs]))),
        ]);

      loop(Fun.id);
    }
  )
  |> lexeme;