pub mod build;
mod engine;
pub mod format;
mod resolve;

pub enum TargetFormat {
    JavaScript(js::Module),
}

enum Command {
    Build(TargetFormat),
}
