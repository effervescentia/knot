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

pub enum Module {
    /// ECMAScript modules
    ESM,

    /// CommonJS modules
    CJS,
}

pub struct Options {
    pub mode: Mode,
    pub module: Module,
}

pub fn generate(module: &ModuleShape) -> JavaScript {
    JavaScript::from_module(
        module,
        &Options {
            mode: Mode::Prod,
            module: Module::ESM,
        },
    )
}
