pub mod build;
mod engine;
pub mod format;
mod resolve;

pub enum TargetFormat {
    JavaScript(knot_generate_javascript::Module),
}

enum Command {
    Build(TargetFormat),
}
