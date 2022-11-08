open Knot.Kore;

let _to_point = (cursor, decoder) =>
  cursor
    ? Point.create(Uutf.decoder_line(decoder), Uutf.decoder_col(decoder))
    : Point.zero;

let _to_stream = (cursor, decoder) =>
  Stream.from(_ =>
    switch (Uutf.decode(decoder)) {
    | `Uchar(uchar) =>
      Some(Input.create(uchar, _to_point(cursor, decoder)))
    | `Malformed(_) =>
      Some(Input.create(Uutf.u_rep, _to_point(cursor, decoder)))
    | `End => None
    | `Await => assert(false)
    }
  );

let _decoder = input =>
  Uutf.decoder(
    ~nln=`Readline(Uchar.of_char(Constants.Character.eol)),
    input,
  );

/**
 wrapper to ingest a stream of unicode characters
 */
type t = Stream.t(Input.t);

/* static */

let of_string = (~cursor=true) => Decoder.of_string % _to_stream(cursor);

let of_channel = (~cursor=true) => Decoder.of_channel % _to_stream(cursor);

/* methods */

let __initial = Point.zero;

let scan = (predicate: Node.t(string, unit) => bool, contents: string) => {
  let stream = of_string(contents);
  let buffer = Buffer.create(8);

  let rec loop = (start, end_) => {
    switch (Stream.peek(stream)) {
    | Some((uchar, cursor)) =>
      Stream.junk(stream);

      switch (Uchar.to_char(uchar)) {
      | ' '
      | '\n'
      | '\r'
      | '\t' =>
        if (Buffer.length(buffer) == 0) {
          loop(cursor, cursor);
        } else {
          let token = buffer |> Buffer.contents;
          let node = Node.untyped(token, Range.create(start, end_));

          buffer |> Buffer.clear;

          if (predicate(node)) {
            Some(node);
          } else {
            loop(start, end_);
          };
        }
      | _ =>
        let is_first_char = Buffer.length(buffer) == 0;
        Buffer.add_utf_8_uchar(buffer, uchar);

        loop(is_first_char ? cursor : start, cursor);
      };

    | None => None
    };
  };

  loop(__initial, __initial);
};

/* pretty printing */

let pp: Fmt.t(t) =
  (ppf, stream: t) => {
    let buffer = Buffer.create(Stream.count(stream));

    stream |> Stream.iter(fst % Buffer.add_utf_8_uchar(buffer));

    Fmt.buffer(ppf, buffer);
  };
