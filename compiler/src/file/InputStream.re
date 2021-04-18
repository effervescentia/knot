open Kore;

let _to_cursor = (cursor, decoder) =>
  cursor
    ? Cursor.point(Uutf.decoder_line(decoder), Uutf.decoder_col(decoder))
    : Cursor.zero;

let _to_stream = (cursor, decoder) =>
  Stream.from(_ =>
    switch (Uutf.decode(decoder)) {
    | `Uchar(uchar) =>
      Some(Input.create(uchar, _to_cursor(cursor, decoder)))
    | `Malformed(uchar) =>
      Some(Input.create(Uutf.u_rep, _to_cursor(cursor, decoder)))
    | `End => None
    | `Await => assert(false)
    }
  );

/**
 wrapper to ingest a stream of unicode characters
 */
type t = Stream.t(Input.t);

/* static */

let of_string = (~cursor=true, s: string): t =>
  decoder(`String(s)) |> _to_stream(cursor);

let of_channel = (~cursor=true, channel: in_channel): t =>
  decoder(`Channel(channel)) |> _to_stream(cursor);

let to_string = (stream: t): string => {
  let buffer = Buffer.create(stream |> Stream.count);

  stream |> Stream.iter(fst % Buffer.add_utf_8_uchar(buffer));

  buffer |> Buffer.contents;
};

let __initial = Cursor.{line: 0, column: 0};

let scan = (predicate: Block.t(string) => bool, contents: string) => {
  let stream = of_string(contents);
  let buffer = Buffer.create(8);

  let rec loop = (start, end_) => {
    switch (stream |> Stream.peek |?> Tuple.map_snd2(Cursor.expand % fst)) {
    | Some((uchar, cursor)) =>
      stream |> Stream.junk;

      switch (uchar |> Uchar.to_char) {
      | ' '
      | '\n'
      | '\r'
      | '\t' =>
        if (Buffer.length(buffer) == 0) {
          loop(cursor, cursor);
        } else {
          let token = buffer |> Buffer.contents;
          let block = Block.create(token, Cursor.Range(start, end_));

          buffer |> Buffer.clear;

          if (predicate(block)) {
            Some(block);
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
