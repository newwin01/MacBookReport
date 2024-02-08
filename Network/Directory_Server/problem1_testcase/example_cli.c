uments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s arg1 arg2\n", argv[0]);
        return 1;
    }

    // Retrieve and print the two arguments
    char *arg1 = argv[1];
    char *arg2 = argv[2];

    printf("Argument 1: %s\n", arg1);
    printf("Argument 2: %s\n", arg2);

    return 0;
}
                                                                                                                                                                                                                                                  