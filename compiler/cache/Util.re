open Core;

let pipe = (buffer_size, in_channel, out_channel) => {
  let channel_length = in_channel_length(in_channel);

  let rec write = () => {
    let remaining = channel_length - pos_in(in_channel);
    let buf_length = min(buffer_size, remaining);
    let buf = Bytes.create(buf_length);

    let read = input(in_channel, buf, 0, buf_length);
    output(out_channel, buf, 0, read);

    if (remaining != 0) {
      write();
    };
  };

  write();

  flush(out_channel);
};
