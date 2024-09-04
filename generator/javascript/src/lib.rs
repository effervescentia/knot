mod ast;
mod format;
mod generate;
mod resolve;
#[cfg(test)]
mod test;
mod transform;

pub use ast::JavaScript;
pub use generate::Generator;
use kore::internal;
use lang::ast as knot;
use resolve::Resolver;

#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord)]
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

pub struct Options<Library> {
    pub mode: internal::Mode,
    pub resolver: Resolver<Library>,
}
