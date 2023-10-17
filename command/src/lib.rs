pub mod build;
mod engine;
pub mod format;
mod resolve;

pub enum TargetFormat {
    JavaScript(gen_js::Module),
}

enum Command {
    Build(TargetFormat),
}
