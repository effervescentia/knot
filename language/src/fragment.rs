use crate::{ast, NodeId, ScopeId};
use std::collections::BTreeMap;

#[derive(Clone, Debug, PartialEq)]
pub enum Fragment {
    Expression(ast::Expression<NodeId, NodeId, NodeId>),
    Statement(ast::Statement<NodeId>),
    Attribute(ast::Attribute<NodeId>),
    Component(ast::Component<NodeId, NodeId, NodeId>),
    Parameter(ast::Parameter<String, NodeId, NodeId>),
    Declaration(ast::Declaration<String, NodeId, NodeId, NodeId, NodeId>),
    TypeExpression(ast::TypeExpression<String, NodeId>),
    Import(ast::Import),
    Module(ast::Module<NodeId, NodeId>),
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
                path, alias: None, ..
            }) => path.last().cloned().map(|x| (x, None)),

            Self::Import(ast::Import {
                alias: Some(alias), ..
            }) => Some((alias.clone(), None)),

            _ => None,
        }
    }
}

pub type FragmentMap<T> = BTreeMap<T, (ScopeId, Fragment)>;
