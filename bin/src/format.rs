use crate::{
    config::Config,
    path::get_root_dir,
    reporter::{eprint_configuration, Phase},
};
use command::format;
use std::path::Path;

pub struct Args<'a> {
    pub glob: &'a str,
    pub root_dir: &'a Path,
}

impl<'a> Args<'a> {
    fn report(&self) {
        let Self { root_dir, glob } = self;

        eprint_configuration(vec![
            ("root_dir", Config::Path(root_dir)),
            ("glob", Config::String(glob)),
        ]);
    }
}

#[allow(clippy::needless_pass_by_value)]
pub fn command(args: Args) -> engine::Result<()> {
    args.report();

    let root_dir = get_root_dir(args.root_dir)?;

    eprint!("{}", Phase::Execution);

    format::command(&format::Options {
        glob: args.glob,
        root_dir: &root_dir,
    })
}
