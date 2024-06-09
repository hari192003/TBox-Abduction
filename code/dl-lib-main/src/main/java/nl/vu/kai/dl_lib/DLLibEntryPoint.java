package nl.vu.kai.dl_lib;

import nl.vu.kai.dl_lib.datatypes.DLHelpers$;
import nl.vu.kai.dl_lib.datatypes.Ontology;
import nl.vu.kai.dl_lib.owlapi.OWLParser;
import nl.vu.kai.dl_lib.owlapi.OWLExporter;
import nl.vu.kai.dl_lib.owlapi.OWLApiConverter;
import nl.vu.kai.dl_lib.formatting.SimpleDLFormatterCl;
import nl.vu.kai.dl_lib.reasoning.DLReasoner;
import nl.vu.kai.dl_lib.reasoning.DLReasoners;
import py4j.GatewayServer;
import scala.Option;

public class DLLibEntryPoint {

    public DLLibEntryPoint() {
    }

    public OWLParser getOWLParser() {
        return new OWLParser();
    }


    public OWLParser getOWLParser(boolean simplifiedNames) {
        return new OWLParser(new OWLApiConverter(simplifiedNames, Option.empty()));
    }

    public void convertToBinaryConjunctions(Ontology ontology) {
        DLHelpers$.MODULE$.convert2binaryConjunctions(ontology);
    }

    public OWLExporter getOWLExporter() { return new OWLExporter(false); }

    public SimpleDLFormatterCl getSimpleDLFormatter() { return new SimpleDLFormatterCl(true); }

    public ELFactory$ getELFactory() {
        return ELFactory$.MODULE$;
    }

    public DLFactory$ getDLFactory() { return DLFactory$.MODULE$; }

    public String getType(Object object) {
        return object.getClass().getSimpleName();
    }

    public DLReasoner getELKReasoner() { return DLReasoners.getELKReasoner(); }

    public DLReasoner getHermiTReasoner() { return DLReasoners.getHermiTReasoner(); }

    public static void main(String[] args) {
        int portNumber = -1;
        if(args.length>0){
            portNumber = Integer.parseInt(args[0]);
            System.out.println("Trying port number "+portNumber);
        }

        GatewayServer gatewayServer;
        if(portNumber==-1)
            gatewayServer = new GatewayServer(new nl.vu.kai.dl_lib.DLLibEntryPoint());
        else
            gatewayServer = new GatewayServer(new nl.vu.kai.dl_lib.DLLibEntryPoint(), portNumber);
        gatewayServer.start();
        System.out.println("Gateway server for DL-Lib started");
    }
}
