open Kore;
open AST;
open Reference;
open Util.ResultUtil;

let suite =
  "Library.AST"
  >::: [
    "Dump.pp() - prim"
    >: (
      () =>
        [
          (
            "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=nil_const />
    <Constant@0.0 type=nil>
      <Primitive@0.0 type=nil>
        <Nil@0.0 type=nil />
      </Primitive@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
            Fixtures.nil_const |> ~@Dump.pp,
          ),
          (
            "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=int_const />
    <Constant@0.0 type=int>
      <Primitive@0.0 type=int>
        <Number@0.0 type=int value=123 />
      </Primitive@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
            Fixtures.int_const |> ~@Dump.pp,
          ),
          (
            "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=float_const />
    <Constant@0.0 type=float>
      <Primitive@0.0 type=float>
        <Number@0.0 type=float value=123.000 />
      </Primitive@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
            Fixtures.float_const |> ~@Dump.pp,
          ),
          (
            "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=bool_const />
    <Constant@0.0 type=bool>
      <Primitive@0.0 type=bool>
        <Boolean@0.0 type=bool value=true />
      </Primitive@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
            Fixtures.bool_const |> ~@Dump.pp,
          ),
          (
            "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=string_const />
    <Constant@0.0 type=string>
      <Primitive@0.0 type=string>
        <String@0.0 type=string value=\"foo\" />
      </Primitive@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
            Fixtures.string_const |> ~@Dump.pp,
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "Dump.pp() - expr"
    >: (
      () =>
        [
          (
            "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=identifier_const />
    <Constant@0.0 type=int>
      <Identifier@0.0 type=int value=foo />
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
            Fixtures.identifier_const |> ~@Dump.pp,
          ),
          (
            "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=jsx_const />
    <Constant@0.0 type=element>
      <JSX@0.0 type=element>
        <Tag@0.0 type=element>
          <Name@0.0 value=Foo />
          <Attributes />
          <Children />
        </Tag@0.0>
      </JSX@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
            Fixtures.jsx_const |> ~@Dump.pp,
          ),
          (
            "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=group_const />
    <Constant@0.0 type=int>
      <Group@0.0 type=int>
        <Primitive@0.0 type=int>
          <Number@0.0 type=int value=123 />
        </Primitive@0.0>
      </Group@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
            Fixtures.group_const |> ~@Dump.pp,
          ),
          (
            "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=closure_const />
    <Constant@0.0 type=bool>
      <Closure@0.0 type=bool>
        <Statement@0.0 type=nil>
          <Variable@0.0 type=nil>
            <Name@0.0 value=foo />
            <Primitive@0.0 type=int>
              <Number@0.0 type=int value=123 />
            </Primitive@0.0>
          </Variable@0.0>
        </Statement@0.0>
        <Statement@0.0 type=nil>
          <Variable@0.0 type=nil>
            <Name@0.0 value=bar />
            <Primitive@0.0 type=nil>
              <Nil@0.0 type=nil />
            </Primitive@0.0>
          </Variable@0.0>
        </Statement@0.0>
        <Statement@0.0 type=bool>
          <Expression@0.0 type=bool>
            <Primitive@0.0 type=bool>
              <Boolean@0.0 type=bool value=false />
            </Primitive@0.0>
          </Expression@0.0>
        </Statement@0.0>
      </Closure@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
            Fixtures.closure_const |> ~@Dump.pp,
          ),
          (
            "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=and_bool_const />
    <Constant@0.0 type=bool>
      <And@0.0 type=bool>
        <LHS@0.0 type=bool>
          <Primitive@0.0 type=bool>
            <Boolean@0.0 type=bool value=true />
          </Primitive@0.0>
        </LHS@0.0>
        <RHS@0.0 type=bool>
          <Primitive@0.0 type=bool>
            <Boolean@0.0 type=bool value=false />
          </Primitive@0.0>
        </RHS@0.0>
      </And@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
            Fixtures.and_bool_const |> ~@Dump.pp,
          ),
          (
            "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=negative_int_const />
    <Constant@0.0 type=int>
      <Negative@0.0 type=int>
        <Primitive@0.0 type=int>
          <Number@0.0 type=int value=123 />
        </Primitive@0.0>
      </Negative@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
            Fixtures.negative_int_const |> ~@Dump.pp,
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "Dump.pp() - jsx"
    >: (
      () =>
        [
          (
            "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=complex_jsx_const />
    <Constant@0.0 type=element>
      <JSX@0.0 type=element>
        <Tag@0.0 type=element>
          <Name@0.0 value=Foo />
          <Attributes>
            <ID@0.0 type=string>
              <Name@0.0 value=bar />
            </ID@0.0>
            <Class@0.0 type=string>
              <Name@0.0 value=fizz />
            </Class@0.0>
            <Property@0.0 type=Abstract<Unknown>>
              <Name@0.0 value=buzz />
            </Property@0.0>
          </Attributes>
          <Children>
            <Node@0.0 type=element>
              <Tag@0.0 type=element>
                <Name@0.0 value=Bar />
                <Attributes />
                <Children />
              </Tag@0.0>
            </Node@0.0>
            <InlineExpr@0.0 type=nil>
              <Primitive@0.0 type=nil>
                <Nil@0.0 type=nil />
              </Primitive@0.0>
            </InlineExpr@0.0>
            <Text@0.0 type=string value=fizzbuzz />
            <Node@0.0 type=element>
              <Fragment@0.0 type=element />
            </Node@0.0>
          </Children>
        </Tag@0.0>
      </JSX@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
            Fixtures.complex_jsx_const |> ~@Dump.pp,
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "Dump.pp() - function"
    >: (
      () =>
        [
          (
            "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=inline_function />
    <Function@0.0 type=Function<(foo: int, bar: int), int>>
      <Arguments>
        <Argument@0.0 type=int>
          <Name@0.0 value=foo />
        </Argument@0.0>
        <Argument@0.0 type=int>
          <Primitive@0.0 type=int>
            <Number@0.0 type=int value=3 />
          </Primitive@0.0>
          <Name@0.0 value=bar />
        </Argument@0.0>
      </Arguments>
      <Body>
        <Add@0.0 type=int>
          <LHS@0.0 type=int>
            <Identifier@0.0 type=int value=foo />
          </LHS@0.0>
          <RHS@0.0 type=int>
            <Identifier@0.0 type=int value=bar />
          </RHS@0.0>
        </Add@0.0>
      </Body>
    </Function@0.0>
  </Declaration@0.0>
</AST>",
            Fixtures.inline_function |> ~@Dump.pp,
          ),
          (
            "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=multiline_function />
    <Function@0.0 type=Function<(foo: int, bar: int), int>>
      <Arguments />
      <Body>
        <Closure@0.0 type=int>
          <Statement@0.0 type=nil>
            <Variable@0.0 type=nil>
              <Name@0.0 value=zip />
              <Primitive@0.0 type=int>
                <Number@0.0 type=int value=3 />
              </Primitive@0.0>
            </Variable@0.0>
          </Statement@0.0>
          <Statement@0.0 type=nil>
            <Variable@0.0 type=nil>
              <Name@0.0 value=zap />
              <Primitive@0.0 type=int>
                <Number@0.0 type=int value=4 />
              </Primitive@0.0>
            </Variable@0.0>
          </Statement@0.0>
          <Statement@0.0 type=int>
            <Expression@0.0 type=int>
              <Mult@0.0 type=int>
                <LHS@0.0 type=int>
                  <Identifier@0.0 type=int value=zip />
                </LHS@0.0>
                <RHS@0.0 type=int>
                  <Identifier@0.0 type=int value=zap />
                </RHS@0.0>
              </Mult@0.0>
            </Expression@0.0>
          </Statement@0.0>
        </Closure@0.0>
      </Body>
    </Function@0.0>
  </Declaration@0.0>
</AST>",
            Fixtures.multiline_function |> ~@Dump.pp,
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "Dump.pp() - import"
    >: (
      () =>
        [
          (
            "<AST>
  <Import@0.0 namespace=main_import>
    <MainImport@0.0>
      <Name@0.0 value=Foo />
    </MainImport@0.0>
  </Import@0.0>
</AST>",
            Fixtures.main_import |> ~@Dump.pp,
          ),
          (
            "<AST>
  <Import@0.0 namespace=named_import>
    <NamedImport@0.0>
      <Name@0.0 value=foo />
    </NamedImport@0.0>
  </Import@0.0>
</AST>",
            Fixtures.named_import |> ~@Dump.pp,
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "Dump.pp() - program"
    >: (() => [("<AST />", [] |> ~@Dump.pp)] |> Assert.(test_many(string))),
  ];
