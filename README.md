# DataBases

ΥΣΒΔ ΕΡΓΑΣΙΑ 

Ομαδική:

1) Νικόλαος Λεπίδας		ΑΜ: 1115201600090
2) Γεώργιος Λιακόπουλος		ΑΜ: 1115201600091


HT_CreateIndex:

Η συνάρτηση αυτή δεσμεύει τα απαιτούμενα blocks και δημιουργεί το primary hash table . Στο πρώτο block (block με αριθμό 0) βρίσκονται οι πληροφορίες του hash table , στην συνέχεια δεσμεύται και ένα block για κάθε bucket . Θεωρούμε ότι το block με αριθμό 1 αντιρποσωπεύει το bucket 1 κοκ έως το number of buckets.


HT_InsertEnty :

Η συνάρτηση αυτή βρίσκει μέσω της intHashing( universal hashing) ή μέσω της stringHashing(αν είναι κλειδί κάποιο εκ των name,surname,address) , το bucket στο οποίο θα γίνει η εισαγωγή του καινούριου record. Διαβάζει το πρώτο block του συγκεκριμένου bucket και αν δεν είναι γεμάτο βάζει το καινούριο record εκεί.
Αν αυτό το block είναι γεμάτο διαβάζει το επόμενο block του ίδιου bucket κοκ μέχρι να βρει block με διαθέσιμο χώρο για το record. Αν και το τελευταίο block του bucket είναι γεμάτο , τότε δεσμεύει ένα καινούριο block , κάνει το προηγούμενο γεμάτο block να δείχνει στο καινούριο block και τοποθετεί το record στο καινούριο block.

HT_GetAllEntries:

Η συνάρτηση αυτή βρίσκει μέσω της intHashing ή μέσω της stringHashing(αν είναι κλειδί κάποιο εκ των name,surname,address) , το bucket στο οποίο είναι το κλειδί-id που αναζητά. Στην συνέχεια διαβάζει τα blocks του συγκεκριμένου bucket μέχρι να βρει το ζητούμενο id και να εκτυπώσει ολόκληρο το record.

HT_DeleteEntry: 

Η συνάρτηση αυτή βρίσκει μέσω της intHashing ή μέσω της stringHashing(αν είναι κλειδί κάποιο εκ των name,surname,address) , το bucket στο οποίο βρίσκεται το κλειδί-id που αναζητά για να διαγραφεί το αντίστοιχο record. Στην συνέχεια διαβάζει τα blocks του συγκεκριμένου bucket μέχρι να βρει το ζητούμενο id . Μόλις το βρει παίρνει το τελευταίο record του συγκεκριμένου block και το τοποθετεί στην θέση του record προς διαγραφή. Μετά αρχικοποιεί σε κενό το record που ήταν πριν στην τελευταία θέση.

SHT_CreateSecondaryIndex:

Η συνάρτηση αυτή δημιουργεί το secondary hash table με την ίδια λογική όπως η HT_CreateIndex . Επιπλέον , αν το primary hash table έχει records τότε για κάθε ένα από αυτά τα records φτιάχνει το αντίστοιχο secondary record του και το εισάγει στο secondary hash table . Δηλαδή εκτελεί και μια λειτουργία συγχρονισμού. 

SHT_SecondaryInsertEntry: 

Η συνάρτηση αυτή ελέγχει αρχικά αν το secondary record που πρόκειται να κάνει εισαγωγή στο secondary hash table υπάρχει και στο primary hash table. Αν δεν υπάρχει τότε επιστρέφει σφάλμα. Αν υπάρχει τότε δουλεύει παρόμοια με την HT_InsertEntry. Ωστόσο δεν χρησιμοποιεί ως κλειδί το id , ως κλειδί χρησιμοποιείται ένα εκ των name , surname , address .

SHT_SecondaryGetAllEntries:

Η συνάρτηση αυτή βρίσκει μέσω της stringHashing το bucket στο οποίο είναι το κλειδί-id που αναζητά. Στην συνέχεια διαβάζει τα blocks του συγκεκριμένου bucket στο secondary hash table μέχρι να βρει το ζητούμενο κλειδί. Όταν το βρει διαβάζει το block στο οποίο βρίσκεται το κλειδί στο primary hashtable κι εκτυπώνει όλα τα records που έχουν το κλειδί . Χρησιμοποιείται ένας πίνακας όπου κρατούνται τα ids των blocks του primary hash table που έχουν γίνει visited , προκειμένου να μην διαβάσουμε κάποιο block δεύτερη φορά και εκτυπώσουμε τα ίδια records.
