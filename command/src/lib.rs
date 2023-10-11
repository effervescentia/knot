pub mod format;
pub mod transpile;

pub enum TargetFormat {
    JavaScript(knot_generate_javascript::Module),
}

enum Command {
    Transpile(TargetFormat),
}
