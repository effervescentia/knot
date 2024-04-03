use super::walk::{self, WalkEach};
use crate::Range;
use std::fmt::Debug;

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum ImportSource {
    Root,
    Local,
    Named(String),
    Scoped { scope: String, name: String },
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Import {
    pub source: ImportSource,
    pub path: Vec<String>,
    pub alias: Option<String>,
}

impl Import {
    pub fn new(source: ImportSource, path: Vec<String>, alias: Option<String>) -> Self {
        Self {
            source,
            path,
            alias,
        }
    }
}

impl<Visitor> walk::Walk<Visitor, Range> for walk::Span<Import>
where
    Visitor: walk::Visit<Range>,
{
    type Output = Visitor::Import;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let Self(
            super::Import {
                source,
                path,
                alias,
            },
            range,
        ) = self;

        v.import(
            super::Import {
                source,
                path,
                alias,
            },
            range,
        )
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Module<Import, Declaration> {
    pub imports: Vec<Import>,
    pub declarations: Vec<Declaration>,
}

impl<Import, Declaration> Module<Import, Declaration> {
    pub fn new(imports: Vec<Import>, declarations: Vec<Declaration>) -> Self {
        Self {
            imports,
            declarations,
        }
    }
}

impl<Visitor, Import, Declaration> walk::Walk<Visitor, Range>
    for walk::Span<Module<Import, Declaration>>
where
    Visitor: walk::Visit<Range>,
    Import: walk::Walk<Visitor, Range, Output = Visitor::Import>,
    Declaration: walk::Walk<Visitor, Range, Output = Visitor::Declaration>,
{
    type Output = Visitor::Module;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let Self(
            super::Module {
                imports,
                declarations,
            },
            range,
        ) = self;
        let ((imports, declarations), v) = (imports, declarations).walk_each(v);

        v.module(
            super::Module {
                imports,
                declarations,
            },
            range,
        )
    }
}
