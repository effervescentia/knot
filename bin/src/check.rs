use command::check;
use std::path::PathBuf;

pub struct Args {
    pub entry: PathBuf,
    pub root_dir: PathBuf,
    pub source_dir: PathBuf,
}

pub fn command(
    Args {
        ref entry,
        ref root_dir,
        ref source_dir,
    }: Args,
) -> engine::Result<()> {
    check::command(&check::Options {
        entry,
        root_dir,
        source_dir,
    })
}
