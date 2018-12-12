var data = [];
var nrNodes;

function preload() {
    bootUp()
    tmp = 1;
    for (let index =0;  index < topo.nodes.length; index++) {
        
        data[index] = loadStrings(topo.nodes[index].dpid);         
        tmp++;
        
    }
    console.log(data)
 
   
}


function setup() {
 
  
  
  creatTable();
  
}

function timedRefresh(timeoutPeriod) {
	setTimeout("location.reload(true);",timeoutPeriod);
}

window.onload = timedRefresh(10000);


var topo = {
    nodes: [],
   
    node_index: {}, // dpid -> index of nodes array
    initialize: function (data) {
        this.add_nodes(data.switches);
       
    },
    add_nodes: function (nodes) {
        for (var i = 0; i < nodes.length; i++) {
            this.nodes.push(nodes[i]);
        }
        this.refresh_node_index();
    },
    
    delete_nodes: function (nodes) {
        for (var i = 0; i < nodes.length; i++) {
            

            node_index = this.get_node_index(nodes[i]);
            this.nodes.splice(node_index, 1);
        }
        this.refresh_node_index();
    },
    
    get_node_index: function (node) {
        for (var i = 0; i < this.nodes.length; i++) {
            if (node.dpid == this.nodes[i].dpid) {
                return i;
            }
        }
        return null;
    },

    get_node_name: function (i) {
        
        return this.nodes[i].name;
    },

       
    
    refresh_node_index: function(){
        this.node_index = {};
        for (var i = 0; i < this.nodes.length; i++) {
            this.node_index[this.nodes[i].dpid] = i;
        }
    },
}



function bootUp(params) {
    var ws = new WebSocket("ws://" + location.host + "/v1.0/topology/ws");
ws.onmessage = function(event) {
    var data = JSON.parse(event.data);

    var result = rpc[data.method](data.params);

    var ret = {"id": data.id, "jsonrpc": "2.0", "result": result};
   
}

d3.json("/v1.0/topology/switches", function(error, switches) {
  
   

});
}

function creatTable(){
   
    var table = document.getElementById("table");
     
    for (let nodeI = 0; nodeI < topo.nodes.length; nodeI++) {
        console.log(topo.nodes[nodeI].ports.length);

       for (let portI = 0; portI < topo.nodes[nodeI].ports.length; portI++) {
        var row = table.insertRow(portI+1);
        var id = row.insertCell(0); 
        var name = row.insertCell(1);
        var mac = row.insertCell(2);

        id.innerHTML = topo.nodes[nodeI].ports[portI].dpid;      
        name.innerHTML = topo.nodes[nodeI].ports[portI].name;
        mac.innerHTML = topo.nodes[nodeI].ports[portI].hw_addr;
        //console.log(topo.nodes[nodeI].ports[portI].name);

        tdata = data[nodeI];

        for (let columI = 0; columI < 7; columI++) {
            var cell1 = row.insertCell(columI+3);
        
            cell1.innerHTML = tdata[ (portI)*7+columI ];
            //console.log( ( (portI)*7+columI) )
            //console.log(tdata[ (portI)*7+columI ])
        }
    
       }
        
    }
    
}




