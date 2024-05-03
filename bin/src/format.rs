use command::format;
use std::path::PathBuf;

pub struct Args {
    pub glob: String,
    pub root_dir: PathBuf,
}

pub fn command(
    Args {
        ref glob,
        ref root_dir,
    }: Args,
) -> engine::Result<()> {
    format::command(&format::Options { glob, root_dir })
}
