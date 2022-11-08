open Kore;
open Onyx;

module Character = Constants.Character;

let double_quote = char(Character.double_quote);
let period = char(Character.period);
let underscore = char(Character.underscore);

let space = one_of([' ', '\t', '\n']);
let spaces = skip_many(space);

let digit = range('0', '9');

let lower_alpha = range('a', 'z');
let upper_alpha = range('A', 'Z');
let alpha = lower_alpha <|> upper_alpha;

let alpha_num = digit <|> alpha;

let lexeme = x => spaces >> x;

let between = (l, r, x) =>
  map3(
    (l', x', r') => Node.untyped(x', Node.join_ranges(l', r')),
    l,
    x,
    r,
  );

let binary_op = (lx, op, rx) => map3((l, _, r) => (l, r), lx, op, rx);

let rec unary_op = (x, op) => op >>= (f => unary_op(x, op) >|= f) <|> x;

/**
 matches a single character
 */
let symbol = x => char(x) >|= Input.to_node |> lexeme;

/**
 matches a pattern that may be terminated by a semicolon
 */
let terminated = x => x << (Character.semicolon |> symbol |> optional);

/**
 matches a pattern that is separated by commas and may be terminated with a comma
 */
let comma_sep = x =>
  x |> sep_by(symbol(Character.comma)) << optional(symbol(Character.comma));

/**
 matches a pattern that is separated by vertical bars and may start with a vertical bar
 */
let vertical_bar_sep = x =>
  optional(symbol(Character.vertical_bar))
  >> (x |> sep_by(symbol(Character.vertical_bar)));

/**
 matches an assignment operation
 */
let assign = (id, x) => binary_op(id, symbol(Character.equal_sign), x);

/**
 matches an attribution operation
 */
let attribute = (key, value) =>
  binary_op(key, symbol(Character.colon), value);

let between_braces = x =>
  between(symbol(Character.open_brace), symbol(Character.close_brace), x);

let between_parentheses = x =>
  between(symbol(Character.open_paren), symbol(Character.close_paren), x);

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
          >|= (end_ => Node.untyped((), Range.create(start, end_)))
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
          >|= (end_ => Node.untyped(s, Range.create(start, end_)))
        | [c, ...cs] => char(c) >> loop(cs);

      loop(s |> String.to_seq |> List.of_seq) <<! (alpha_num <|> underscore);
    }
  )
  |> lexeme;

/**
 matches a sequence of alpha-numeric characters
 */
let identifier = (~prefix=alpha <|> underscore, input) =>
  (prefix <~> (alpha_num <|> underscore |> many) >|= Input.join |> lexeme)(
    input,
  );

/**
 matches a sequence of characters between quotation marks

 allows escaping characters with a '\' character
 */
let string =
  double_quote
  >|= Input.get_point
  >>= (
    start => {
      let rec loop = f =>
        choice([
          /* end of string sequence */
          double_quote
          >|= Input.get_point
          >|= (
            end_ =>
              Node.untyped(
                f([]) |> String.of_uchars,
                Range.create(start, end_),
              )
          ),
          /* capture escaped characters */
          char(Character.back_slash)
          >> any
          >|= Input.get_value
          >>= (
            c =>
              loop(rs => f([Uchar.of_char(Character.back_slash), c, ...rs]))
          ),
          /* capture characters of the string */
          none_of([Character.double_quote, Character.eol])
          >|= Input.get_value
          >>= (c => loop(rs => f([c, ...rs]))),
        ]);

      loop(Fun.id);
    }
  )
  |> lexeme;
