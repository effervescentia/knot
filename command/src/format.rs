use knot_language::parser;
use std::{
    fs::{self, File},
    io::{BufWriter, Write},
    path::Path,
};

pub struct Options<'a> {
    pub out_dir: &'a Path,
    pub entry: &'a Path,
}

pub fn command(opts: &Options) {
    let input = fs::read_to_string(&opts.entry).unwrap();
    let (result, _) = parser::parse(&input).unwrap();

    let mut output_path = opts.out_dir.to_path_buf();
    output_path.extend(opts.entry.file_name());

    let out_file = File::create(output_path).unwrap();
    let mut writer = BufWriter::new(out_file);

    write!(writer, "{result}").unwrap();
    writer.flush().unwrap();
}
