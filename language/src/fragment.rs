use crate::{ast, NodeId, ScopeId};
use std::collections::BTreeMap;

#[derive(Clone, Debug, PartialEq)]
pub enum Fragment {
    Expression(ast::Expression<NodeId, NodeId, NodeId>),
    Statement(ast::Statement<NodeId>),
    Component(ast::Component<NodeId, NodeId>),
    Parameter(ast::Parameter<String, NodeId, NodeId>),
    Declaration(ast::Declaration<String, NodeId, NodeId, NodeId, NodeId>),
    TypeExpression(ast::TypeExpression<NodeId>),
    Import(ast::Import),
    Module(ast::Module<NodeId, NodeId>),
}

impl Fragment {
    pub fn to_binding(&self) -> Option<String> {
        match self {
            Self::Statement(ast::Statement::Variable(binding, ..))
            | Self::Parameter(ast::Parameter { binding, .. }) => Some(binding.clone()),

            Self::Declaration(x) => Some(x.binding().clone()),

            Self::Import(ast::Import {
                path, alias: None, ..
            }) => path.last().cloned(),

            Self::Import(ast::Import {
                alias: Some(alias), ..
            }) => Some(alias.clone()),

            _ => None,
        }
    }
}

pub type FragmentMap = BTreeMap<NodeId, (ScopeId, Fragment)>;
