use crate::{
    analyzer::{
        context::{BindingMap, FileContext, FragmentMap, ScopeContext, StrongContext, WeakContext},
        fragment::{self, Fragment},
        infer::{strong::StrongRef, weak::WeakRef},
    },
    ast::{
        declaration::{Declaration, DeclarationNode},
        expression::ExpressionNode,
        ksx::KSXNode,
        module::ModuleNode,
        statement::StatementNode,
        type_expression::TypeExpressionNode,
    },
    common::{node::Node, range::Range},
    parser::CharStream,
};
use std::{
    cell::RefCell,
    collections::{BTreeMap, BTreeSet, HashMap},
};

const RANGE: Range<CharStream> = Range::chars((1, 1), (1, 1));

type InitRange = ((i32, i32), (i32, i32));

pub fn file_ctx() -> RefCell<FileContext> {
    RefCell::new(FileContext::new())
}

pub fn file_ctx_from(xs: Vec<(usize, (Vec<usize>, Fragment))>) -> FileContext {
    let ctx = file_ctx();
    ctx.borrow_mut().fragments.0.extend(xs);
    ctx.into_inner()
}

pub fn scope_ctx<'a>(file_ctx: &'a RefCell<FileContext>) -> ScopeContext<'a> {
    ScopeContext::new(file_ctx)
}

pub fn strong_ctx_from(
    fragments: Vec<(usize, (Vec<usize>, Fragment))>,
    refs: Vec<(usize, StrongRef)>,
    bindings: Vec<((Vec<usize>, String), BTreeSet<usize>)>,
) -> StrongContext {
    let mut ctx = StrongContext::new(
        FragmentMap(BTreeMap::from_iter(fragments)),
        BindingMap(HashMap::from_iter(bindings)),
    );
    ctx.refs.extend(refs);
    ctx
}

/// node factories
pub mod n {
    use crate::ast::{
        declaration, expression, ksx, module,
        parameter::{self, ParameterNode},
        statement, type_expression,
    };

    use super::*;

    pub fn x(
        x: expression::NodeValue<CharStream<'static>, ()>,
    ) -> ExpressionNode<CharStream<'static>, ()> {
        ExpressionNode::raw(x, RANGE)
    }

    pub fn xr<'a>(
        x: expression::NodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> ExpressionNode<CharStream<'a>, ()> {
        ExpressionNode::raw(x, Range::chars(start, end))
    }

    pub fn xc<T>(
        x: expression::NodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> ExpressionNode<CharStream<'static>, T> {
        ExpressionNode(Node::new(x, RANGE, ctx))
    }

    pub fn s(
        x: statement::NodeValue<CharStream<'static>, ()>,
    ) -> StatementNode<CharStream<'static>, ()> {
        StatementNode::raw(x, RANGE)
    }

    pub fn sr<'a>(
        x: statement::NodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> StatementNode<CharStream<'a>, ()> {
        StatementNode::raw(x, Range::chars(start, end))
    }

    pub fn sc<T>(
        x: statement::NodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> StatementNode<CharStream<'static>, T> {
        StatementNode(Node::new(x, RANGE, ctx))
    }

    pub fn kx(x: ksx::NodeValue<CharStream<'static>, ()>) -> KSXNode<CharStream<'static>, ()> {
        KSXNode::raw(x, RANGE)
    }

    pub fn kxr<'a>(
        x: ksx::NodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> KSXNode<CharStream<'a>, ()> {
        KSXNode::raw(x, Range::chars(start, end))
    }

    pub fn kxc<T>(
        x: ksx::NodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> KSXNode<CharStream<'static>, T> {
        KSXNode(Node::new(x, RANGE, ctx))
    }

    pub fn p(
        x: parameter::NodeValue<CharStream<'static>, ()>,
    ) -> ParameterNode<CharStream<'static>, ()> {
        ParameterNode::raw(x, RANGE)
    }

    pub fn pr<'a>(
        x: parameter::NodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> ParameterNode<CharStream<'a>, ()> {
        ParameterNode::raw(x, Range::chars(start, end))
    }

    pub fn pc<T>(
        x: parameter::NodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> ParameterNode<CharStream<'static>, T> {
        ParameterNode(Node::new(x, RANGE, ctx))
    }

    pub fn tx(
        x: type_expression::NodeValue<CharStream<'static>, ()>,
    ) -> TypeExpressionNode<CharStream<'static>, ()> {
        TypeExpressionNode::raw(x, RANGE)
    }

    pub fn txr<'a>(
        x: type_expression::NodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> TypeExpressionNode<CharStream<'a>, ()> {
        TypeExpressionNode::raw(x, Range::chars(start, end))
    }

    pub fn txc<T>(
        x: type_expression::NodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> TypeExpressionNode<CharStream<'static>, T> {
        TypeExpressionNode(Node::new(x, RANGE, ctx))
    }

    pub fn d(
        x: declaration::NodeValue<CharStream<'static>, ()>,
    ) -> DeclarationNode<CharStream<'static>, ()> {
        DeclarationNode::raw(x, RANGE)
    }

    pub fn dr<'a>(
        x: declaration::NodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> DeclarationNode<CharStream<'a>, ()> {
        DeclarationNode::raw(x, Range::chars(start, end))
    }

    pub fn dc<T>(
        x: declaration::NodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> DeclarationNode<CharStream<'static>, T> {
        DeclarationNode(Node::new(x, RANGE, ctx))
    }

    pub fn mr<'a>(x: module::NodeValue<CharStream<'a>, ()>) -> ModuleNode<CharStream<'a>, ()> {
        ModuleNode::raw(x)
    }
}

/// ast factories
pub mod a {
    use super::*;
    use crate::ast::storage::{Storage, Visibility};

    pub fn type_<E, P, M, T>(name: &str, value: T) -> Declaration<E, P, M, T> {
        Declaration::TypeAlias {
            name: Storage(Visibility::Public, name.to_string()),
            value,
        }
    }

    pub fn enum_<E, P, M, T>(
        name: &str,
        variants: Vec<(String, Vec<T>)>,
    ) -> Declaration<E, P, M, T> {
        Declaration::Enumerated {
            name: Storage(Visibility::Public, name.to_string()),
            variants,
        }
    }

    pub fn const_<E, P, M, T>(
        name: &str,
        value_type: Option<T>,
        value: E,
    ) -> Declaration<E, P, M, T> {
        Declaration::Constant {
            name: Storage(Visibility::Public, name.to_string()),
            value_type,
            value,
        }
    }

    pub fn func_<E, P, M, T>(
        name: &str,
        parameters: Vec<P>,
        body_type: Option<T>,
        body: E,
    ) -> Declaration<E, P, M, T> {
        Declaration::Function {
            name: Storage(Visibility::Public, name.to_string()),
            parameters,
            body_type,
            body,
        }
    }

    pub fn view<E, P, M, T>(name: &str, parameters: Vec<P>, body: E) -> Declaration<E, P, M, T> {
        Declaration::View {
            name: Storage(Visibility::Public, name.to_string()),
            parameters,
            body,
        }
    }

    pub fn mod_<E, P, M, T>(name: &str, value: M) -> Declaration<E, P, M, T> {
        Declaration::Module {
            name: Storage(Visibility::Public, name.to_string()),
            value,
        }
    }
}
