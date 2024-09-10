use crate::{
    ast::{self, ImportSource},
    NodeId, ScopeId,
};
use std::collections::BTreeMap;

pub type FragmentMap<T> = BTreeMap<T, (ScopeId, Fragment)>;

#[derive(Clone, Debug, PartialEq)]
pub enum Fragment {
    /* program */
    Expression(ast::Expression<NodeId, NodeId, NodeId>),
    Statement(ast::Statement<NodeId>),
    Attribute(ast::Attribute<NodeId>),
    Component(ast::Component<NodeId, NodeId, NodeId>),
    Parameter(ast::Parameter<String, NodeId, NodeId>),
    Declaration(ast::Declaration<String, NodeId, NodeId, NodeId, NodeId>),
    TypeExpression(ast::TypeExpression<String, NodeId>),
    Import(ast::Import),
    Module(ast::Module<NodeId, NodeId>),

    /* typings */
    TypeDeclaration(ast::TypeDeclaration<String, NodeId, NodeId>),
    TypeModule(ast::TypeModule<NodeId, NodeId>),
}

impl Fragment {
    pub fn to_binding(&self) -> Option<(String, Option<NodeId>)> {
        match self {
            Self::Statement(ast::Statement::Variable(binding, x)) => {
                Some((binding.clone(), Some(*x)))
            }

            Self::Parameter(ast::Parameter { binding, .. }) => Some((binding.clone(), None)),

            Self::Declaration(x) => Some((x.binding().clone(), None)),

            Self::Import(ast::Import {
                source: ImportSource::Named(name),
                path,
                alias: None,
            }) if path.is_empty() => Some((name.to_owned(), None)),

            Self::Import(ast::Import {
                path, alias: None, ..
            }) => path.last().cloned().map(|x| (x, None)),

            Self::Import(ast::Import {
                alias: Some(alias), ..
            }) => Some((alias.clone(), None)),

            Self::TypeDeclaration(x) => Some((x.binding().clone(), None)),

            _ => None,
        }
    }
}
