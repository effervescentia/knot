open Knot.Kore;
open Parse.Onyx;

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

let terminated = x => x << (Character.semicolon |> lexeme |> optional);

let between = (l, r, x) =>
  map3(
    (l', x', r') =>
      Block.create(Cursor.join(Block.cursor(l'), Block.cursor(r')), x'),
    l,
    x,
    r,
  );

let binary_op = (lx, op, rx) => map3((l, _, r) => (l, r), lx, op, rx);

let rec unary_op = (x, op) => op >>= (f => unary_op(x, op) >|= f) <|> x;

/**
 * matches a single character
 */
let symbol = x => char(x) >|= Char.to_block |> lexeme;

/**
 * matches a sequence of characters but tolerates spaces in between
 */
let glyph = (s: string) =>
  ctx
  >>= (
    start => {
      let rec loop =
        fun
        | [] => assert(false)
        | [c] =>
          char(c)
          >|= Char.context
          >|= (end_ => Block.create(Cursor.join(start, end_), ()))
          |> lexeme
        | [c, ...cs] => char(c) |> lexeme >> loop(cs);

      s |> String.to_seq |> List.of_seq |> loop;
    }
  )
  |> lexeme;

/**
 * matches a sequence of characters
 */
let keyword = (s: string) =>
  ctx
  >>= (
    start => {
      let rec loop =
        fun
        | [] => assert(false)
        | [c] =>
          char(c)
          >|= Char.context
          >|= (end_ => Block.create(Cursor.join(start, end_), ()))
        | [c, ...cs] => char(c) >> loop(cs);

      loop(s |> String.to_seq |> List.of_seq);
    }
  )
  |> lexeme;

/**
 * matches a sequence of alpha-numeric characters
 */
let identifier = (~prefix=alpha <|> Character.underscore, input) =>
  (
    prefix
    <~> (alpha_num <|> Character.underscore |> many)
    >|= Char.join
    |> lexeme
  )(
    input,
  );

/**
 * matches a sequence of characters between quotation marks
 * allows escaping characters with a '\' character
 */
let string =
  Character.quote
  >|= Char.context
  >>= (
    start => {
      let rec loop = f =>
        choice([
          /* end of string sequence */
          Character.quote
          >|= Char.context
          >|= (
            end_ =>
              Block.create(
                Cursor.join(start, end_),
                f([]) |> String.of_uchars,
              )
          ),
          /* capture escaped characters */
          Character.back_slash
          >> any
          >|= Char.value
          >>= (c => loop(rs => f([__back_slash, c, ...rs]))),
          /* capture characters of the string */
          none_of([C.Character.quote, C.Character.eol])
          >|= Char.value
          >>= (c => loop(rs => f([c, ...rs]))),
        ]);

      loop(Functional.identity);
    }
  )
  |> lexeme;
