echo "Copying modified files into ns-3 instillation at $1"
cp -v ns-3-mod/global-router-interface.* $1/ns-3.19/src/internet/model
cp -v ns-3-mod/ipv4-routing-table-entry.* $1/ns-3.19/src/internet/model
cp -v ns-3-mod/ipv4-global-routing.* $1/ns-3.19/src/internet/model
cp -v ns-3-mod/ipv4-header.* $1/ns-3.19/src/internet/model
cp -v ns-3-mod/second.cc $1/ns-3.19/examples/tutorial


