include Knot.Kore;

let decoder = input =>
  Uutf.decoder(
    ~nln=`Readline(Uchar.of_char(Constants.Character.eol)),
    input,
  );
