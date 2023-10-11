use crate::{
    analyzer::{
        context::{BindingMap, FileContext, FragmentMap, ScopeContext, StrongContext},
        fragment::Fragment,
        infer::strong::StrongRef,
    },
    ast::{
        Declaration, DeclarationNode, ExpressionNode, KSXNode, ModuleNode, StatementNode,
        TypeExpressionNode,
    },
    common::{node::Node, range::Range},
    parser::CharStream,
};
use std::{
    cell::RefCell,
    collections::{BTreeMap, BTreeSet, HashMap},
};

#[allow(dead_code)]
const RANGE: Range<CharStream> = Range::chars((1, 1), (1, 1));

#[allow(dead_code)]
type InitRange = ((i32, i32), (i32, i32));

#[allow(dead_code)]
pub fn file_ctx() -> RefCell<FileContext> {
    RefCell::new(FileContext::new())
}

#[allow(dead_code)]
pub fn file_ctx_from(xs: Vec<(usize, (Vec<usize>, Fragment))>) -> FileContext {
    let ctx = file_ctx();
    ctx.borrow_mut().fragments.0.extend(xs);
    ctx.into_inner()
}

#[allow(dead_code)]
pub fn scope_ctx<'a>(file_ctx: &'a RefCell<FileContext>) -> ScopeContext<'a> {
    ScopeContext::new(file_ctx)
}

#[allow(dead_code)]
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
#[allow(dead_code)]
pub mod n {
    use super::*;
    use crate::ast::{
        DeclarationNodeValue, ExpressionNodeValue, KSXNodeValue, ModuleNodeValue, ParameterNode,
        ParameterNodeValue, StatementNodeValue, TypeExpressionNodeValue,
    };

    pub fn x(
        x: ExpressionNodeValue<CharStream<'static>, ()>,
    ) -> ExpressionNode<CharStream<'static>, ()> {
        ExpressionNode::raw(x, RANGE)
    }

    pub fn xr<'a>(
        x: ExpressionNodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> ExpressionNode<CharStream<'a>, ()> {
        ExpressionNode::raw(x, Range::chars(start, end))
    }

    pub fn xc<T>(
        x: ExpressionNodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> ExpressionNode<CharStream<'static>, T> {
        ExpressionNode(Node::new(x, RANGE, ctx))
    }

    pub fn s(
        x: StatementNodeValue<CharStream<'static>, ()>,
    ) -> StatementNode<CharStream<'static>, ()> {
        StatementNode::raw(x, RANGE)
    }

    pub fn sr<'a>(
        x: StatementNodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> StatementNode<CharStream<'a>, ()> {
        StatementNode::raw(x, Range::chars(start, end))
    }

    pub fn sc<T>(
        x: StatementNodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> StatementNode<CharStream<'static>, T> {
        StatementNode(Node::new(x, RANGE, ctx))
    }

    pub fn kx(x: KSXNodeValue<CharStream<'static>, ()>) -> KSXNode<CharStream<'static>, ()> {
        KSXNode::raw(x, RANGE)
    }

    pub fn kxr<'a>(
        x: KSXNodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> KSXNode<CharStream<'a>, ()> {
        KSXNode::raw(x, Range::chars(start, end))
    }

    pub fn kxc<T>(
        x: KSXNodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> KSXNode<CharStream<'static>, T> {
        KSXNode(Node::new(x, RANGE, ctx))
    }

    pub fn p(
        x: ParameterNodeValue<CharStream<'static>, ()>,
    ) -> ParameterNode<CharStream<'static>, ()> {
        ParameterNode::raw(x, RANGE)
    }

    pub fn pr<'a>(
        x: ParameterNodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> ParameterNode<CharStream<'a>, ()> {
        ParameterNode::raw(x, Range::chars(start, end))
    }

    pub fn pc<T>(
        x: ParameterNodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> ParameterNode<CharStream<'static>, T> {
        ParameterNode(Node::new(x, RANGE, ctx))
    }

    pub fn tx(
        x: TypeExpressionNodeValue<CharStream<'static>, ()>,
    ) -> TypeExpressionNode<CharStream<'static>, ()> {
        TypeExpressionNode::raw(x, RANGE)
    }

    pub fn txr<'a>(
        x: TypeExpressionNodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> TypeExpressionNode<CharStream<'a>, ()> {
        TypeExpressionNode::raw(x, Range::chars(start, end))
    }

    pub fn txc<T>(
        x: TypeExpressionNodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> TypeExpressionNode<CharStream<'static>, T> {
        TypeExpressionNode(Node::new(x, RANGE, ctx))
    }

    pub fn d(
        x: DeclarationNodeValue<CharStream<'static>, ()>,
    ) -> DeclarationNode<CharStream<'static>, ()> {
        DeclarationNode::raw(x, RANGE)
    }

    pub fn dr<'a>(
        x: DeclarationNodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> DeclarationNode<CharStream<'a>, ()> {
        DeclarationNode::raw(x, Range::chars(start, end))
    }

    pub fn dc<T>(
        x: DeclarationNodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> DeclarationNode<CharStream<'static>, T> {
        DeclarationNode(Node::new(x, RANGE, ctx))
    }

    pub fn mr<'a>(x: ModuleNodeValue<CharStream<'a>, ()>) -> ModuleNode<CharStream<'a>, ()> {
        ModuleNode::raw(x)
    }
}

/// ast factories
#[allow(dead_code)]
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

    pub fn module<E, P, M, T>(name: &str, value: M) -> Declaration<E, P, M, T> {
        Declaration::Module {
            name: Storage(Visibility::Public, name.to_string()),
            value,
        }
    }
}
