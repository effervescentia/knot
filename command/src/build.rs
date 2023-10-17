use super::TargetFormat;
use lang::ast::ToShape;
use std::{
    fs::{self, File},
    io::{BufWriter, Write},
    path::Path,
};

pub struct Options<'a> {
    pub target: TargetFormat,
    pub out_dir: &'a Path,
    pub entry: &'a Path,
}

pub fn command(opts: &Options) {
    let input = fs::read_to_string(&opts.entry).unwrap();
    let (ast, _) = parse::parse(&input).unwrap();
    let typed = analyze::analyze(ast);

    let shape = typed.to_shape();
    let result = gen_js::generate(&shape);

    let mut output_path = opts.out_dir.to_path_buf();
    output_path.extend(opts.entry.file_name().and_then(|x| {
        let replace = x.to_str()?.replace(".kn", ".js");
        Some(Path::new(&replace).to_path_buf())
    }));

    let mut writer = BufWriter::new(File::create(output_path).unwrap());
    write!(writer, "{result}").unwrap();
    writer.flush().unwrap();
}
