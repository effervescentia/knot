include Knot.Kore;

let create = input =>
  Uutf.decoder(
    ~nln=`Readline(Uchar.of_char(Constants.Character.eol)),
    input,
  );

let of_string = x => create(`String(x));

let of_channel = x => create(`Channel(x));
