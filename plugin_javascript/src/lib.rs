mod format;
mod javascript;
mod transform;

pub use javascript::JavaScript;
use lang::ast::ProgramShape;

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

impl Default for Module {
    fn default() -> Self {
        Self::ESM
    }
}

pub struct Options {
    pub mode: Mode,
    pub module: Module,
}

pub fn generate(program: &ProgramShape) -> JavaScript {
    JavaScript::from_module(
        &program.0,
        &Options {
            mode: Mode::Prod,
            module: Module::ESM,
        },
    )
}
