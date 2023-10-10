mod format;
mod javascript;
mod transform;

use javascript::JavaScript;
use knot_language::ast::ModuleShape;

// TODO: move these to a common library to be re-used across generators

#[derive(PartialEq)]
pub enum Mode {
    Dev,
    Prod,
}

pub struct Options {
    mode: Mode,
}

pub fn generate(module: &ModuleShape) -> JavaScript {
    JavaScript::from_module(module, &Options { mode: Mode::Prod })
}
